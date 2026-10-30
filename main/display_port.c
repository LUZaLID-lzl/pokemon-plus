#include "display_port.h"
#include "display_transfer.h"
#include "bsp/esp-bsp.h"
#include "bsp/touch.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lv_adapter.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "touch_input.h"
/* Waveshare 1.85B Rev1.1 schematic, J3 TE net: LCD_TE(GPIO18).
 * The original BSP initialization tables already enable TE with command 0x35.
 * No LCD command, bus clock, coordinate transform or vendor source is changed. */
#define PP_LCD_TE_GPIO GPIO_NUM_18
static const char *TAG="pp-display";
static esp_lcd_touch_handle_t touch;
static TaskHandle_t touch_task;
static pp_touch_input samples;
static portMUX_TYPE sample_lock=portMUX_INITIALIZER_UNLOCKED;
static bool input_enabled=true;
static SemaphoreHandle_t te_signal;
static StaticSemaphore_t te_storage;
static uint32_t te_edges;
static unsigned sync_misses,read_errors;
static int64_t sync_retry_at,refresh_started;
static uint32_t refresh_max_us;
static bool frame_sync_pending;
/* Keep LVGL's full-capacity PSRAM render buffer, but never submit it directly
 * to SPI: that path allocates a DMA bounce buffer for every queued chunk.
 * Each staged rectangle fits ONE board-configured SPI transaction. */
static esp_lcd_panel_handle_t lcd_panel;
static uint8_t *dma_pixels;
static size_t dma_capacity;
static SemaphoreHandle_t dma_done;
static StaticSemaphore_t dma_done_storage;
static unsigned transfer_errors,transfer_count;
static bool redraw_pending;

static bool IRAM_ATTR color_done(esp_lcd_panel_io_handle_t io,
    esp_lcd_panel_io_event_data_t *data,void *ctx) {
  (void)io;(void)data;(void)ctx;
  BaseType_t wake=pdFALSE;xSemaphoreGiveFromISR(dma_done,&wake);
  return wake==pdTRUE;
}
static bool send_pixels(void *ctx,unsigned y,unsigned rows,const uint8_t *pixels,size_t bytes) {
    const lv_area_t *area=ctx;
    (void)bytes;
    xSemaphoreTake(dma_done,0);
    esp_err_t err=esp_lcd_panel_draw_bitmap(lcd_panel,area->x1,area->y1+y,
        area->x2+1,area->y1+y+rows,pixels);
    if(err!=ESP_OK) {
      /* One transaction per rectangle: failed submission has no earlier
       * queued pieces still referencing this buffer. Retry the whole screen. */
      ESP_LOGE(TAG,"LCD rectangle failed (%s); full repaint scheduled",esp_err_to_name(err));
      return false;
    }
    /* Never reuse DMA memory or release the LVGL buffer before color_done.
     * A lost completion is a driver fault: fail visibly instead of silently
     * corrupting later frames or leaving the UI locked indefinitely. */
    ESP_ERROR_CHECK(xSemaphoreTake(dma_done,pdMS_TO_TICKS(1000))==pdTRUE?ESP_OK:ESP_ERR_TIMEOUT);
    portENTER_CRITICAL(&sample_lock);transfer_count++;portEXIT_CRITICAL(&sample_lock);
    return true;
}
static void flush_pixels(lv_display_t *display,const lv_area_t *area,uint8_t *pixels) {
  if(!pp_display_transfer(lv_area_get_width(area),lv_area_get_height(area),pixels,
      dma_pixels,dma_capacity,send_pixels,(void *)area)) {
    portENTER_CRITICAL(&sample_lock);transfer_errors++;portEXIT_CRITICAL(&sample_lock);
    redraw_pending=true;
    lv_timer_resume(lv_display_get_refr_timer(display));
    lv_timer_ready(lv_display_get_refr_timer(display));
  }
  lv_display_flush_ready(display);
}

static void IRAM_ATTR touch_irq(esp_lcd_touch_handle_t handle) {
  (void)handle;
  if(touch_task) {
    BaseType_t wake=pdFALSE;vTaskNotifyGiveFromISR(touch_task,&wake);
    if(wake)portYIELD_FROM_ISR();
  }
}
static void sample_touch(void *arg) {
  (void)arg;
  bool down=false;
  int64_t last_success=esp_timer_get_time();
  for(;;) {
    /* A press wakes immediately; while held, poll for moves AND release.
     * LVGL rendering/TE waits never prevent this task from capturing edges. */
    uint32_t notified=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(PP_TOUCH_POLL_MS));
    if(!notified && !down)continue;
    esp_lcd_touch_point_data_t point={0};uint8_t count=0;
    esp_err_t err=esp_lcd_touch_read_data(touch);
    if(err==ESP_OK)err=esp_lcd_touch_get_data(touch,&point,&count,1);
    if(err!=ESP_OK) {
      portENTER_CRITICAL(&sample_lock);read_errors++;portEXIT_CRITICAL(&sample_lock);
      /* Brief I2C failures are not releases; persistent failure must unstick. */
      if(esp_timer_get_time()-last_success<100000)continue;
      count=0;
    } else last_success=esp_timer_get_time();
    down=count>0;
    portENTER_CRITICAL(&sample_lock);
    if(input_enabled)pp_touch_sample(&samples,down,point.x,point.y);
    portEXIT_CRITICAL(&sample_lock);
  }
}
static void read_touch(lv_indev_t *indev,lv_indev_data_t *data) {
  bool more;
  portENTER_CRITICAL(&sample_lock);
  pp_touch_point p=pp_touch_read(&samples,&more);
  portEXIT_CRITICAL(&sample_lock);
  data->point.x=p.x;data->point.y=p.y;
  data->state=p.down?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
  data->continue_reading=more;
  /* A preceding queued release may have rebuilt the page. Resolve its layout
   * before LVGL hit-tests this press, even without an intervening refresh. */
  if(p.down)lv_obj_update_layout(lv_display_get_screen_active(lv_indev_get_display(indev)));
}
static void IRAM_ATTR te_irq(void *arg) {
  (void)arg;BaseType_t wake=pdFALSE;
  portENTER_CRITICAL_ISR(&sample_lock);te_edges++;portEXIT_CRITICAL_ISR(&sample_lock);
  xSemaphoreGiveFromISR(te_signal,&wake);
  if(wake)portYIELD_FROM_ISR();
}
static void display_event(lv_event_t *event) {
  lv_event_code_t code=lv_event_get_code(event);
  if(code==LV_EVENT_INVALIDATE_AREA) {
    lv_area_t *area=lv_event_get_param(event);
    area->x1&=~1;area->x2|=1;
  } else if(code==LV_EVENT_REFR_START) {
    refresh_started=esp_timer_get_time();frame_sync_pending=true;
    if(redraw_pending) {
      redraw_pending=false;
      lv_obj_invalidate(lv_display_get_screen_active(lv_event_get_target(event)));
    }
  } else if(code==LV_EVENT_FLUSH_START && frame_sync_pending) {
    frame_sync_pending=false;
    if(te_signal && esp_timer_get_time()>=sync_retry_at) {
      /* Once per dirty refresh, rather than once per pet or DMA chunk.
       * Discard a stale edge; wait at most 25ms for the next real scan cycle. */
      xSemaphoreTake(te_signal,0);
      if(xSemaphoreTake(te_signal,pdMS_TO_TICKS(PP_DISPLAY_SYNC_TIMEOUT_MS))!=pdTRUE) {
        portENTER_CRITICAL(&sample_lock);sync_misses++;portEXIT_CRITICAL(&sample_lock);
        sync_retry_at=esp_timer_get_time()+(int64_t)PP_DISPLAY_SYNC_RETRY_MS*1000;
        if(sync_misses==1)ESP_LOGW(TAG,"No LCD TE edge; refresh continues without sync, retry in 1s");
      }
    }
  } else if(code==LV_EVENT_REFR_READY) {
    uint32_t duration=(uint32_t)(esp_timer_get_time()-refresh_started);
    portENTER_CRITICAL(&sample_lock);
    if(duration>refresh_max_us)refresh_max_us=duration;
    portEXIT_CRITICAL(&sample_lock);
  }
}
static void setup_sync(void) {
  te_signal=xSemaphoreCreateBinaryStatic(&te_storage);
  gpio_config_t config={.pin_bit_mask=1ULL<<PP_LCD_TE_GPIO,.mode=GPIO_MODE_INPUT};
  esp_err_t err=gpio_config(&config);
  if(err==ESP_OK) {
    /* Use the same idle-level edge selection as the installed Espressif adapter. */
    err=gpio_set_intr_type(PP_LCD_TE_GPIO,gpio_get_level(PP_LCD_TE_GPIO)?GPIO_INTR_NEGEDGE:GPIO_INTR_POSEDGE);
  }
  if(err==ESP_OK) {err=gpio_install_isr_service(0);if(err==ESP_ERR_INVALID_STATE)err=ESP_OK;}
  if(err==ESP_OK)err=gpio_isr_handler_add(PP_LCD_TE_GPIO,te_irq,NULL);
  if(err!=ESP_OK) {te_signal=NULL;ESP_LOGW(TAG,"TE unavailable: %s; refresh remains enabled",esp_err_to_name(err));}
}
lv_display_t *pp_display_start(lv_indev_t **input) {
  *input=NULL;
  esp_lv_adapter_config_t adapter=ESP_LV_ADAPTER_DEFAULT_CONFIG();
  if(esp_lv_adapter_init(&adapter)!=ESP_OK)return NULL;
  bsp_display_config_t board={0};esp_lcd_panel_handle_t panel=NULL;esp_lcd_panel_io_handle_t io=NULL;
  if(bsp_display_new(&board,&panel,&io)!=ESP_OK)return NULL;
  esp_lv_adapter_display_config_t config={.panel=panel,.panel_io=io,
    .profile={.interface=ESP_LV_ADAPTER_PANEL_IF_OTHER,.rotation=ESP_LV_ADAPTER_ROTATE_0,
      .hor_res=BSP_LCD_H_RES,.ver_res=BSP_LCD_V_RES,
      /* One full-capacity PSRAM draw buffer prevents a 48px pet + old location
       * being split at the former 50-row draw-buffer boundary. Still partial. */
      .buffer_height=BSP_LCD_V_RES,.use_psram=true,.require_double_buffer=false},
    .tear_avoid_mode=ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE};
  lv_display_t *display=esp_lv_adapter_register_display(&config);
  if(!display)return NULL;
  size_t max_bytes=0;
  if(spi_bus_get_max_transaction_len(BSP_LCD_SPI_NUM,&max_bytes)!=ESP_OK)return NULL;
  dma_capacity=LV_MIN(max_bytes,4092)&~(size_t)3;
  if(dma_capacity<BSP_LCD_H_RES*2)return NULL;
  dma_pixels=heap_caps_malloc(dma_capacity,MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA);
  if(!dma_pixels)return NULL;
  lcd_panel=panel;dma_done=xSemaphoreCreateBinaryStatic(&dma_done_storage);
  esp_lcd_panel_io_callbacks_t callbacks={.on_color_trans_done=color_done};
  if(esp_lcd_panel_io_register_event_callbacks(io,&callbacks,NULL)!=ESP_OK)return NULL;
  lv_display_set_flush_cb(display,flush_pixels);
  bsp_display_cfg_t board_touch={.touch_flags={.swap_xy=0,.mirror_x=0,.mirror_y=0}};
  if(bsp_touch_new(&board_touch,&touch)!=ESP_OK)return NULL;
  if(esp_lcd_touch_register_interrupt_callback(touch,touch_irq)!=ESP_OK)goto fail;
  if(esp_lv_adapter_lock(-1)!=ESP_OK)goto fail;
  *input=lv_indev_create();
  if(!*input) {esp_lv_adapter_unlock();goto fail;}
  lv_indev_set_type(*input,LV_INDEV_TYPE_POINTER);
  lv_indev_set_disp(*input,display);lv_indev_set_read_cb(*input,read_touch);
  lv_timer_set_period(lv_indev_get_read_timer(*input),PP_TOUCH_POLL_MS);
  setup_sync();lv_display_add_event_cb(display,display_event,LV_EVENT_ALL,NULL);
  esp_lv_adapter_unlock();
  if(bsp_display_brightness_init()!=ESP_OK)goto fail;
  if(xTaskCreate(sample_touch,"pp-touch",3072,NULL,adapter.task_priority+1,&touch_task)!=pdPASS)goto fail;
  /* Read the initial state, even if an IRQ arrived before the task existed. */
  xTaskNotifyGive(touch_task);
  if(esp_lv_adapter_start()!=ESP_OK)goto fail;
  ESP_LOGI(TAG,"Touch edges queued at 10ms; PSRAM render 259200 bytes; fixed DMA %u bytes; bounded GPIO18 TE sync",(unsigned)dma_capacity);
  return display;
fail:
  esp_lcd_touch_register_interrupt_callback(touch,NULL);
  if(touch_task) {vTaskDelete(touch_task);touch_task=NULL;}
  gpio_isr_handler_remove(PP_LCD_TE_GPIO);
  *input=NULL;
  return NULL;
}
void pp_display_input_enable(bool enabled) {
  portENTER_CRITICAL(&sample_lock);input_enabled=enabled;pp_touch_reset(&samples);portEXIT_CRITICAL(&sample_lock);
  /* A fresh hardware read also clears wake-up suppression if already released. */
  if(touch_task)xTaskNotifyGive(touch_task);
}
void pp_display_log_stats(void) {
  unsigned presses,releases,overflows,errors,misses,transfers,failed;uint32_t edges,refresh_us;
  portENTER_CRITICAL(&sample_lock);
  presses=samples.presses;releases=samples.releases;overflows=samples.overflows;
  errors=read_errors;misses=sync_misses;edges=te_edges;refresh_us=refresh_max_us;
  transfers=transfer_count;failed=transfer_errors;
  portEXIT_CRITICAL(&sample_lock);
  ESP_LOGI(TAG,"Input down/up=%u/%u overflow=%u I2C errors=%u; TE edges=%lu misses=%u max refresh=%lu us; LCD transfers=%u errors=%u",
    presses,releases,overflows,errors,(unsigned long)edges,misses,(unsigned long)refresh_us,transfers,failed);
}
