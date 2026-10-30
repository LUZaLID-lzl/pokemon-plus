#include "bsp/esp-bsp.h"
#include "button_gpio.h"
#include "driver/usb_serial_jtag.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_sntp.h"

#include "esp_timer.h"
#include "esp_wifi.h"
#include "game_ui.h"
#include "fishing_view.h"
#include "display_port.h"
#include "image_assets.h"
#include "iot_button.h"
#include "legacy_game_v1.h"
#include "legacy_game_v2.h"
#include "legacy_game_v3.h"
#include "legacy_game_v4.h"
#include "legacy_game_v5.h"
#include "legacy_game_v6.h"
#include "legacy_game_v7.h"
#include "legacy_game_v8.h"
#include "legacy_game_v9.h"
#include "legacy_game_v10.h"
#include "legacy_game_v11.h"
#include "pedometer.h"
#include "battery_monitor.h"
#include "rtc_clock.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <math.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
static pp_game game, snapshot;
static struct {
  uint32_t checksum;
  int64_t wall;
  pp_game_v11 game;
} save;
static struct {
  uint32_t checksum;
  int64_t wall;
  pp_game_v1 game;
} legacy;
static struct { uint32_t checksum; int64_t wall; pp_game_v2 game; } legacy_v2;
static struct { uint32_t checksum; int64_t wall; pp_game_v3 game; } legacy_v3;
static struct { uint32_t checksum; int64_t wall; pp_game_v4 game; } legacy_v4;
static struct { uint32_t checksum; int64_t wall; pp_game_v5 game; } legacy_v5;
static struct { uint32_t checksum; int64_t wall; pp_game_v6 game; } legacy_v6;
static struct { uint32_t checksum; int64_t wall; pp_game_v7 game; } legacy_v7;
static struct { uint32_t checksum; int64_t wall; pp_game_v8 game; } legacy_v8;
static struct { uint32_t checksum; int64_t wall; pp_game_v9 game; } legacy_v9;
static struct { uint32_t checksum; int64_t wall; pp_game_v10 game; } legacy_v10;
static const char *TAG = "pokemon-plus";
static void battery_delay(unsigned ms) {vTaskDelay(pdMS_TO_TICKS(ms));}
static void battery_log(void) {
  pp_battery_diagnostics d;
  if(!pp_battery_monitor_diagnostics(&d)) {
    ESP_LOGW(TAG,"Battery diagnostics unavailable");return;
  }
  ESP_LOGI(TAG,"Battery: SOC=%u%% voltage=%u mV current=%d mA average=%d mA temperature=%u dK remaining=%u mAh full=%u mAh design=%u mAh status=0x%04x operation=0x%04x",
    d.soc,d.voltage_mv,d.current_ma,d.average_ma,d.temperature_dk,
    d.remaining_mah,d.full_mah,d.design_mah,d.status,d.operation);
}
static bool save_failed, screen_on = true, intro_stored;
static lv_timer_t *display_timer;
static uint64_t loaded_seconds;
static volatile bool boot_pressed, boot_held;
static int64_t saved_wall;
static bool loaded_current, invalid_current_save;
static bool rtc_available;
static atomic_bool rtc_sync_requested;
static void configure_warehouse_storage(void){
  nvs_stats_t stats;
  if(nvs_get_stats(NULL,&stats)==ESP_OK){
    /* Account for unrelated namespaces (including Wi-Fi); game blobs are replaced
     * and old versions reclaimed after commit. Keep a GC page and both save copies. */
    size_t game_entries=0;nvs_handle_t h;
    if(nvs_open("pokemon_plus",NVS_READONLY,&h)==ESP_OK){
      for(unsigned v=1;v<=PP_VERSION;v++){
        char key[16];snprintf(key,sizeof(key),"save_v%u",v);size_t bytes=0;
        if(nvs_get_blob(h,key,NULL,&bytes)==ESP_OK)game_entries+=(bytes+31)/32+(bytes+3999)/4000+1;
      }
      nvs_close(h);
    }
    size_t other=stats.used_entries>game_entries?stats.used_entries-game_entries:0;
    size_t entries=stats.total_entries>126+other?stats.total_entries-126-other:0;
    pp_set_storage_budget(entries>32?(unsigned)((entries/2-16)*32):0);
  }
}

static bool wall_valid(time_t t) {
  return t >= PP_CLOCK_MIN_EPOCH && t < PP_CLOCK_MAX_EPOCH;
}
static void load_game(void) {
  pp_init(&game, esp_random());
  configure_warehouse_storage();
  pp_ui_display_load(PP_UI_DISPLAY_DEFAULT);
  pp_ui_auto_supply_load(0);
  nvs_handle_t h;
  if (nvs_open("pokemon_plus", NVS_READONLY, &h) != ESP_OK)
    return;
  uint8_t hud=PP_UI_DISPLAY_DEFAULT;
  esp_err_t hud_error=nvs_get_u8(h,"hud_display_v2",&hud);
  if(hud_error==ESP_OK)pp_ui_display_load(hud);
  else if(hud_error==ESP_ERR_NVS_NOT_FOUND && nvs_get_u8(h,"hud_display",&hud)==ESP_OK && hud<=3)
    pp_ui_display_load(hud|PP_UI_SHOW_BATTERY);
  uint8_t auto_supply=0;
  if(nvs_get_u8(h,"training_supply",&auto_supply)==ESP_OK)pp_ui_auto_supply_load(auto_supply);
  uint8_t seen=0;
  intro_stored=nvs_get_u8(h,"intro_seen",&seen)==ESP_OK && seen==1;
  pp_ui_onboarding(intro_stored);
  for(unsigned v=PP_VERSION;v>=12;v--){
    char key[16];snprintf(key,sizeof(key),"save_v%u",v);size_t current_size=0;
    esp_err_t current_err=nvs_get_blob(h,key,NULL,&current_size);
    if(current_err==ESP_ERR_NVS_NOT_FOUND)continue;
    void *blob=current_err==ESP_OK?malloc(current_size):NULL;
    if(blob && nvs_get_blob(h,key,blob,&current_size)==ESP_OK && pp_save_read(&game,blob,current_size,&saved_wall)){
      loaded_current=v==PP_VERSION;game.clock_ready=0;
      ESP_LOGI(TAG,"Loaded v%u warehouse: %u pets",v,pp_warehouse_count(&game));
    }else {save_failed=true;invalid_current_save=true;ESP_LOGE(TAG,"Invalid warehouse save retained");}
    free(blob);nvs_close(h);return;
  }
  size_t size = sizeof(save);
  esp_err_t err = nvs_get_blob(h, "save_v11", &save, &size);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    size_t v10_size=sizeof(legacy_v10);
    esp_err_t v10_err=nvs_get_blob(h,"save_v10",&legacy_v10,&v10_size);
    if(v10_err==ESP_OK && v10_size==sizeof(legacy_v10) &&
       pp_import_v10(&game,&legacy_v10.game,sizeof(legacy_v10.game),legacy_v10.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v10.wall;
      ESP_LOGI(TAG,"Imported v10 save; original blob retained");nvs_close(h);return;
    }
    if(v10_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v10 save retained");nvs_close(h);return;}
    size_t v9_size=sizeof(legacy_v9);
    esp_err_t v9_err=nvs_get_blob(h,"save_v9",&legacy_v9,&v9_size);
    if(v9_err==ESP_OK && v9_size==sizeof(legacy_v9) &&
       pp_import_v9(&game,&legacy_v9.game,sizeof(legacy_v9.game),legacy_v9.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v9.wall;
      ESP_LOGI(TAG,"Imported v9 save; original blob retained");nvs_close(h);return;
    }
    if(v9_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v9 save retained");nvs_close(h);return;}
    size_t v8_size=sizeof(legacy_v8);
    esp_err_t v8_err=nvs_get_blob(h,"save_v8",&legacy_v8,&v8_size);
    if(v8_err==ESP_OK && v8_size==sizeof(legacy_v8) &&
       pp_import_v8(&game,&legacy_v8.game,sizeof(legacy_v8.game),legacy_v8.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v8.wall;
      ESP_LOGI(TAG,"Imported v8 save; original blob retained");nvs_close(h);return;
    }
    if(v8_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v8 save retained");nvs_close(h);return;}
    size_t v7_size=sizeof(legacy_v7);
    esp_err_t v7_err=nvs_get_blob(h,"save_v7",&legacy_v7,&v7_size);
    if(v7_err==ESP_OK && v7_size==sizeof(legacy_v7) &&
       pp_import_v7(&game,&legacy_v7.game,sizeof(legacy_v7.game),legacy_v7.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v7.wall;
      ESP_LOGI(TAG,"Imported v7 save; original blob retained");nvs_close(h);return;
    }
    if(v7_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v7 save retained");nvs_close(h);return;}
    size_t v6_size=sizeof(legacy_v6);
    esp_err_t v6_err=nvs_get_blob(h,"save_v6",&legacy_v6,&v6_size);
    if(v6_err==ESP_OK && v6_size==sizeof(legacy_v6) &&
       pp_import_v6(&game,&legacy_v6.game,sizeof(legacy_v6.game),legacy_v6.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v6.wall;
      ESP_LOGI(TAG,"Imported v6 save with level %u cap; original blob retained",(unsigned)PP_MAX_LEVEL);nvs_close(h);return;
    }
    if(v6_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v6 save retained");nvs_close(h);return;}
    size_t v5_size=sizeof(legacy_v5);
    esp_err_t v5_err=nvs_get_blob(h,"save_v5",&legacy_v5,&v5_size);
    if(v5_err==ESP_OK && v5_size==sizeof(legacy_v5) &&
       pp_import_v5(&game,&legacy_v5.game,sizeof(legacy_v5.game),legacy_v5.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v5.wall;
      ESP_LOGI(TAG,"Imported v5 save; original blob retained");nvs_close(h);return;
    }
    if(v5_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v5 save retained");nvs_close(h);return;}
    size_t v4_size=sizeof(legacy_v4);
    esp_err_t v4_err=nvs_get_blob(h,"save_v4",&legacy_v4,&v4_size);
    if(v4_err==ESP_OK && v4_size==sizeof(legacy_v4) &&
       pp_import_v4(&game,&legacy_v4.game,sizeof(legacy_v4.game),legacy_v4.checksum)) {
      game.clock_ready=0;saved_wall=legacy_v4.wall;
      ESP_LOGI(TAG,"Imported v4 save; original blob retained");nvs_close(h);return;
    }
    if(v4_err!=ESP_ERR_NVS_NOT_FOUND) {ESP_LOGW(TAG,"Invalid v4 save retained");nvs_close(h);return;}
    size_t v3_size = sizeof(legacy_v3);
    esp_err_t v3_err = nvs_get_blob(h, "save_v3", &legacy_v3, &v3_size);
    if (v3_err == ESP_OK && v3_size == sizeof(legacy_v3) &&
        pp_import_v3(&game, &legacy_v3.game, sizeof(legacy_v3.game), legacy_v3.checksum)) {
      game.clock_ready = 0; saved_wall = legacy_v3.wall;
      ESP_LOGI(TAG, "Imported v3 save; original blob retained");
      nvs_close(h); return;
    }
    if (v3_err != ESP_ERR_NVS_NOT_FOUND) {
      ESP_LOGW(TAG, "Invalid v3 save retained; not overwriting it");
      nvs_close(h); return;
    }

    size_t v2_size=sizeof(legacy_v2);
    esp_err_t v2_err=nvs_get_blob(h,"save_v2",&legacy_v2,&v2_size);
    if (v2_err==ESP_OK && v2_size==sizeof(legacy_v2) &&
        pp_import_v2(&game,&legacy_v2.game,sizeof(legacy_v2.game),legacy_v2.checksum)) {
      game.clock_ready=0; saved_wall=legacy_v2.wall;
      ESP_LOGI(TAG,"Imported v2 save; original blob retained");
      nvs_close(h); return;
    }
    if (v2_err!=ESP_ERR_NVS_NOT_FOUND) {
      ESP_LOGW(TAG,"Invalid v2 save retained; not overwriting it");
      nvs_close(h); return;
    }

    size_t old_size = sizeof(legacy);
    esp_err_t old_err = nvs_get_blob(h, "save_v1", &legacy, &old_size);
    if (old_err == ESP_OK && old_size == sizeof(legacy) &&
        pp_import_v1(&game, &legacy.game, sizeof(legacy.game),
                     legacy.checksum)) {
      game.clock_ready = 0;
      saved_wall = legacy.wall;
      ESP_LOGI(TAG, "Imported v1 save; original blob retained");
    }
    nvs_close(h);
    return;
  }
  nvs_close(h);
  if (err == ESP_OK && size == sizeof(save) && pp_import_v11(&game,&save.game,sizeof(save.game),save.checksum)) {
    game.clock_ready = 0;
    saved_wall = save.wall;
  } else if (err != ESP_ERR_NVS_NOT_FOUND)
    ESP_LOGW(TAG, "Invalid save ignored; original NVS blob preserved until "
                  "next successful save");
}
static bool store_game(time_t wall) {
  if(invalid_current_save)return false;
  nvs_handle_t h;
  esp_err_t err = nvs_open("pokemon_plus", NVS_READWRITE, &h);
  if (err != ESP_OK)
    return false;
  unsigned size=pp_save_size(&snapshot);
  void *blob=malloc(size);
  if(!blob || !pp_save_write(&snapshot,blob,size,wall_valid(wall)?wall:0)){free(blob);nvs_close(h);return false;}
  char key[16];snprintf(key,sizeof(key),"save_v%u",PP_VERSION);
  err=nvs_set_blob(h,key,blob,size);free(blob);
  if(err==ESP_OK)err=nvs_commit(h);
  /* Reclaim old version keys only after the complete current snapshot committed. */
  if(err==ESP_OK){for(unsigned v=1;v<PP_VERSION;v++){char key[16];snprintf(key,sizeof(key),"save_v%u",v);nvs_erase_key(h,key);}nvs_commit(h);}
  nvs_close(h);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "Save failed: %s", esp_err_to_name(err));
  if(err==ESP_OK)configure_warehouse_storage();
  return err == ESP_OK;
}
static void clock_synced(struct timeval *tv) {
  (void)tv;
  /* SNTP runs on another task. All RTC I2C stays on the application task. */
  atomic_store(&rtc_sync_requested,true);
}
static void clock_restore_rtc(i2c_master_bus_handle_t bus) {
  esp_err_t err=pp_rtc_clock_init(bus);
  rtc_available=err==ESP_OK;
  int64_t epoch;
  if (rtc_available) err=pp_rtc_clock_read(&epoch);
  if (err==ESP_OK) {
    struct timeval tv={.tv_sec=(time_t)epoch};
    if (settimeofday(&tv,NULL)==0) {
      ESP_LOGI(TAG,"Clock restored from RTC (UTC epoch %lld)",(long long)epoch);
      return;
    }
    err=ESP_FAIL;
  }
  ESP_LOGW(TAG,"RTC time unavailable (%s); waiting for USB/network sync",esp_err_to_name(err));
}
static void clock_persist_rtc(void) {
  static bool pending,attempted;
  static uint64_t attempted_at;
  bool requested=atomic_exchange(&rtc_sync_requested,false);
  if (requested) pending=true;
  uint64_t now=esp_timer_get_time()/1000000;
  if (!pending || !rtc_available || (!requested && attempted && now-attempted_at<30)) return;
  time_t epoch=time(NULL);
  if (!wall_valid(epoch)) return;
  attempted=true;attempted_at=now;
  esp_err_t err=pp_rtc_clock_write((int64_t)epoch);
  if (err==ESP_OK) {
    pending=false;
    ESP_LOGI(TAG,"RTC synchronized and verified (UTC epoch %lld)",(long long)epoch);
  } else ESP_LOGW(TAG,"RTC sync failed (%s); retry in 30s",esp_err_to_name(err));
}
static void wifi_event(void *arg, esp_event_base_t base, int32_t id,
                       void *data) {
  (void)arg;
  (void)data;
  if (base == WIFI_EVENT &&
      (id == WIFI_EVENT_STA_START || id == WIFI_EVENT_STA_DISCONNECTED))
    esp_wifi_connect();
  if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
    if (esp_sntp_enabled())
      return;
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_set_time_sync_notification_cb(clock_synced);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
  }
}
static void clock_start(void) {
  setenv("TZ", "CST-8", 1);
  tzset();
  if (!strlen(CONFIG_PP_WIFI_SSID))
    return;
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&init));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             wifi_event, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             wifi_event, NULL));
  wifi_config_t config = {0};
  snprintf((char *)config.sta.ssid, sizeof(config.sta.ssid), "%s",
           CONFIG_PP_WIFI_SSID);
  snprintf((char *)config.sta.password, sizeof(config.sta.password), "%s",
           CONFIG_PP_WIFI_PASSWORD);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
static char serial_line[64];
static unsigned serial_length;
static void serial_poll(void) {
  uint8_t c;
  while (usb_serial_jtag_read_bytes(&c, 1, 0) > 0) {
    if (c == '\n' || c == '\r') {
      serial_line[serial_length] = 0;
      if (!strncmp(serial_line, "TIME ", 5)) {
        char *end;
        long long seconds = strtoll(serial_line + 5, &end, 10);
        if (*end == 0 && wall_valid(seconds)) {
          struct timeval tv = {.tv_sec = seconds};
          if (settimeofday(&tv, NULL)==0) {
            atomic_store(&rtc_sync_requested,true);
            ESP_LOGI(TAG, "Clock synchronized over USB");
          } else ESP_LOGW(TAG,"USB clock sync failed");
        }
      }
      serial_length = 0;
    } else if (serial_length < sizeof(serial_line) - 1)
      serial_line[serial_length++] = c;
    else
      serial_length = 0;
  }
}
static void boot_edge(void *b, void *arg) {
  (void)b;
  (void)arg;
  boot_pressed = true;
}
static void boot_hold(void *b,void *arg) {(void)b;(void)arg;boot_held=true;}
void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  load_game();
  unsigned loaded_display=pp_ui_display_options();
  bool loaded_auto_supply=pp_ui_auto_supply();
  ESP_LOGI(TAG,"Save ready: v%lu pets=%u coins=%lu total_steps=%llu",(unsigned long)game.version,game.count,(unsigned long)game.coins,(unsigned long long)game.total_steps);
  loaded_seconds = game.seconds;
  configure_warehouse_storage();
  usb_serial_jtag_driver_config_t serial = {.rx_buffer_size = 256,
                                            .tx_buffer_size = 256};
  ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&serial));
  lv_indev_t *input = NULL;
  lv_display_t *display = pp_display_start(&input);
  if (!display || !input) {
    ESP_LOGE(TAG, "Display/touch unavailable; stopping");
    return;
  }
  if (lv_display_get_horizontal_resolution(display) != 360 ||
      lv_display_get_vertical_resolution(display) != 360) {
    ESP_LOGE(TAG, "Cannot confirm display geometry; stopping");
    return;
  }
  bool assets_ok = pp_image_bank_init();
  if (!assets_ok) ESP_LOGE(TAG,"Versioned image bank unavailable or mismatched; reflash matching app and assets package");
  else ESP_LOGI(TAG,"Resource package v%s matches application v%s",pp_image_bank_version(),PP_APP_VERSION);
  static qmi8658_dev_t imu;
  bool bus_ok = bsp_i2c_init() == ESP_OK;
  if (bus_ok) clock_restore_rtc(bsp_i2c_get_handle());
  clock_start();
  bool battery_ok=bus_ok && pp_battery_monitor_init(bsp_i2c_get_handle());
  if(battery_ok) {
    pp_battery_profile_report report;
    int profile=pp_battery_monitor_configure_500mah(battery_delay,&report);
    ESP_LOGI(TAG,"Battery capacity profile: result=%d previous design=%u full=%u target=%u mAh updated=%d rollback=%d stage=%u",
      profile,report.design_before,report.full_before,PP_BATTERY_CAPACITY_MAH,
      report.updated,report.rollback_verified,report.stage);
    if(report.block_saved) {
      ESP_LOGI(TAG,"Battery capacity block before (0x929d, 32 bytes):");
      ESP_LOG_BUFFER_HEX_LEVEL(TAG,report.before,sizeof(report.before),ESP_LOG_INFO);
    }
    if(profile<0) {
      ESP_LOGW(TAG,"Battery capacity configuration failed; SOC display unavailable until restart");
      battery_ok=false;
    }
    battery_log();
  }
  pp_battery_reading battery=battery_ok?pp_battery_monitor_read():(pp_battery_reading){.percent=-1};
  bool imu_ok = bus_ok;
  if (imu_ok)
    imu_ok = qmi8658_init(&imu, bsp_i2c_get_handle(), QMI8658_ADDRESS_HIGH) ==
             ESP_OK;
  if (imu_ok)
    imu_ok = qmi8658_set_accel_range(&imu, QMI8658_ACCEL_RANGE_8G) == ESP_OK;
  if (imu_ok)
    imu_ok = qmi8658_set_accel_odr(&imu, QMI8658_ACCEL_ODR_500HZ) == ESP_OK;
  if (imu_ok) {
    qmi8658_set_accel_unit_mps2(&imu, true);
    imu_ok = qmi8658_write_register(&imu, QMI8658_CTRL5, 0x03) == ESP_OK;
  }
  if (bsp_display_lock(-1) != ESP_OK) {
    return;
  }
  lv_indev_set_long_press_time(input, 1000);
  pp_ui_assets("S:/assets");
  pp_ui_hardware(assets_ok, imu_ok);
  pp_ui_create(&game);
  pp_ui_battery(battery);
  pp_image_stats image_stats = pp_image_probe();
  ESP_LOGI(TAG, "UI ready: cached %u images / %u bytes; indexed Flash images; no filesystem reads",
           image_stats.images, (unsigned)image_stats.bytes);
  if(invalid_current_save)pp_ui_notice("存档读取失败，原档已保留");
  if (!assets_ok) {
    pp_ui_open(11);
    pp_ui_notice("请检查资源包");
  }
  ESP_LOGI(TAG,"LCD DMA heap: free=%u largest=%u",
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),
           (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA));
  bsp_display_unlock();
  ESP_ERROR_CHECK(bsp_display_backlight_on());
  display_timer = lv_display_get_refr_timer(display);
  button_handle_t boot = NULL;
  button_config_t bc = {.long_press_time=1000};
  button_gpio_config_t bg = {.gpio_num = BSP_BUTTONS_IO_0, .active_level = 0};
  if (iot_button_new_gpio_device(&bc, &bg, &boot) == ESP_OK) {
    iot_button_register_cb(boot, BUTTON_SINGLE_CLICK, NULL, boot_edge, NULL);
    iot_button_register_cb(boot, BUTTON_LONG_PRESS_START, NULL, boot_hold, NULL);
  }
  uint64_t last = (uint64_t)(esp_timer_get_time() / 1000000), last_save = last,
           last_shake = 0;
  uint32_t saved_revision = game.revision;
  bool needs_migration_save = !loaded_current;
  uint64_t saved_evolution_declined=game.evolution_declined;
  bool urgent_save=false;
  uint32_t observed_revision=game.revision;
  bool starter_was_pending = pp_needs_starter(&game);
  bool offline_applied = false;
  uint64_t step_report=last, intro_attempt=0, last_battery=last, last_battery_log=last;
  unsigned imu_errors = 0;
  unsigned stored_display=loaded_display;
  bool stored_auto_supply=loaded_auto_supply;
  uint64_t supply_attempt=0;
  uint64_t display_attempt=0;
  float previous_magnitude = ONE_G;
  pp_pedometer pedometer={0};
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(50));
    serial_poll();
    clock_persist_rtc();
    uint64_t now = esp_timer_get_time() / 1000000;
    time_t wall = time(NULL);
    struct tm local;
    localtime_r(&wall, &local);
    bool valid = wall_valid(wall);
    uint32_t day = valid ? (local.tm_year + 1900) * 10000u +
                               (local.tm_mon + 1) * 100u + local.tm_mday
                         : 0;
    bool shake = false;
    unsigned new_steps=0;
    if (imu_ok) {
      bool ready = false;
      qmi8658_data_t raw = {0};
      esp_err_t err = qmi8658_is_data_ready(&imu, &ready);
      if (err == ESP_OK && ready)
        err = qmi8658_read_sensor_data(&imu, &raw);
      if (err != ESP_OK) {
        if (++imu_errors >= 10)
          imu_ok = false;
      } else if (ready) {
        imu_errors = 0;
        float mag = sqrtf(raw.accelX * raw.accelX + raw.accelY * raw.accelY +
                          raw.accelZ * raw.accelZ);
        if (fabsf(mag - previous_magnitude) > ONE_G * 0.9f &&
            now - last_shake >= 1) {
          shake = true;
          last_shake = now;
        }
        previous_magnitude = mag;
        new_steps=pp_pedometer_sample(&pedometer,esp_timer_get_time()/1000,raw.accelX,raw.accelY,raw.accelZ);
      }
    }
    /* Bounded read-only fuel gauge sampling stays outside the display lock. */
    if(now-last_battery>=15){
      battery=battery_ok?pp_battery_monitor_read():(pp_battery_reading){.percent=-1};
      last_battery=now;
    }
    if(bus_ok && now-last_battery_log>=60) {
      battery_log();last_battery_log=now;
    }
    if (bsp_display_lock(-1) != ESP_OK)
      continue;
    pp_ui_battery(battery);
    pp_ui_screen(screen_on);
    /* Preserve an unclicked foe before either offline or live time advances. */
    if(!screen_on)pp_wild_screen(&game,false);
    if (valid && !offline_applied) {
      if (saved_wall > 0 &&
          wall > saved_wall + (time_t)game.seconds - (time_t)loaded_seconds) {
        uint64_t delta = wall - saved_wall - (game.seconds - loaded_seconds);
        if (delta > 7 * 86400)
          delta = 7 * 86400;
        pp_tick(&game, (uint32_t)delta, day);
      }
      offline_applied = true;
    }
    pp_tick(&game, (uint32_t)(now - last), day);
    if(screen_on)pp_wild_screen(&game,true);
    pp_gift_scene(&game,screen_on && pp_ui_page()==0 && !pp_ui_evolving(),(unsigned)(now-last));
    pp_wild_scene(&game,screen_on && pp_ui_page()==0 && !pp_ui_evolving());
    last = now;
    if (shake)
      pp_shake(&game);
    /* UI purchases/care and other non-step changes keep their 2s save cadence.
     * Step-only changes batch at 30s so walking does not constantly write Flash. */
    if (game.revision!=observed_revision) urgent_save=true;
    if (new_steps) pp_record_steps(&game,new_steps);
    observed_revision=game.revision;
    if (boot_held) {
      boot_held=false;
      if(screen_on) {
        uint32_t before=game.revision;pp_ui_boot_hold();
        if(game.revision!=before)urgent_save=true;
      }
    }
    if (boot_pressed) {
      boot_pressed = false;
      screen_on = !screen_on;
      pp_ui_screen(screen_on);
      pp_fishing_view_screen(screen_on);
      pp_wild_screen(&game,screen_on);
      lv_indev_reset(input, NULL);
      pp_display_input_enable(screen_on);
      lv_indev_enable(input, screen_on);
      if (screen_on) {
        if(!pp_ui_evolving() && !game.battling &&
           (game.wild.phase==PP_WILD_PENDING ||
            (game.scene<12 && game.wild.next_at==PP_WILD_STEP_MARK))) {
          pp_wild_scene(&game,true);
          pp_ui_open(0);
        }
        lv_timer_resume(display_timer);
        lv_obj_invalidate(lv_screen_active());
        bsp_display_backlight_on();
      } else {
        bsp_display_backlight_off();
        lv_timer_pause(display_timer);
      }
    }
    if (screen_on) {
      char clock[8] = "--:--";
      if (valid)
        strftime(clock, sizeof(clock), "%H:%M", &local);
      pp_ui_clock(clock);
    }
    pp_ui_hardware(assets_ok, imu_ok);
    bool snapshot_ok=pp_clone(&snapshot,&game);
    /* Permanent refusal and its gift bypass the normal two-second batching. */
    bool decision_pending=game.evolution_declined!=saved_evolution_declined;
    bool need_save = (decision_pending && (!save_failed || now-last_save>=2)) || needs_migration_save || (!pp_needs_starter(&game) && starter_was_pending) ||
                     (now - last_save >= 60) ||
                     (game.revision != saved_revision && now - last_save >= (urgent_save ? 2 : 30));
    bool intro_pending=pp_ui_intro_seen() && !intro_stored;
    unsigned current_display=pp_ui_display_options();
    bool current_auto_supply=pp_ui_auto_supply();
    bsp_display_unlock();
    /* Persist only changed preferences, outside the display lock; retry failures. */
    if(current_display!=stored_display && now-display_attempt>=2) {
      display_attempt=now;nvs_handle_t h;
      if(nvs_open("pokemon_plus",NVS_READWRITE,&h)==ESP_OK) {
        esp_err_t err=nvs_set_u8(h,"hud_display_v2",(uint8_t)current_display);
        if(err==ESP_OK)err=nvs_commit(h);
        nvs_close(h);
        if(err==ESP_OK)stored_display=current_display;
        else ESP_LOGW(TAG,"Display preference save failed; retrying");
      }
    }
    if(current_auto_supply!=stored_auto_supply && now-supply_attempt>=2){
      supply_attempt=now;nvs_handle_t h;
      if(nvs_open("pokemon_plus",NVS_READWRITE,&h)==ESP_OK){
        esp_err_t err=nvs_set_u8(h,"training_supply",current_auto_supply?1:0);
        if(err==ESP_OK)err=nvs_commit(h);
        nvs_close(h);
        if(err==ESP_OK)stored_auto_supply=current_auto_supply;
        else ESP_LOGW(TAG,"Training supply preference save failed; retrying");
      }
    }
    if(intro_pending && now-intro_attempt>=2) {
      intro_attempt=now;
      nvs_handle_t h;
      if(nvs_open("pokemon_plus",NVS_READWRITE,&h)==ESP_OK) {
        esp_err_t err=nvs_set_u8(h,"intro_seen",1);
        if(err==ESP_OK)err=nvs_commit(h);
        nvs_close(h);intro_stored=err==ESP_OK;
        if(!intro_stored)ESP_LOGW(TAG,"Opening flag save failed; retrying");
      }
    }
    if (now-step_report>=30) {
      step_report=now;
      pp_display_log_stats();
      ESP_LOGI(TAG,"Pedometer: IMU=%s today=%lu total=%llu reward=%lu coins/%lu XP per pet",
               imu_ok?"ready":"unavailable",(unsigned long)snapshot.steps_today,
               (unsigned long long)snapshot.total_steps,(unsigned long)snapshot.steps_coins,(unsigned long)snapshot.steps_xp);
    }
    if (need_save) {
      bool ok = snapshot_ok && store_game(wall);
      last_save = now;
      if (ok) {
        needs_migration_save=false;
        urgent_save=false;
        saved_revision = snapshot.revision;
        saved_evolution_declined = snapshot.evolution_declined;
        starter_was_pending = pp_needs_starter(&snapshot);
      }
      if (!ok && !save_failed && bsp_display_lock(-1) == ESP_OK) {
        pp_ui_notice("保存失败，请检查设备存储");
        bsp_display_unlock();
      }
      save_failed = !ok;
    }
  }
}
