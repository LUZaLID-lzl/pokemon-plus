#include "simulator_window.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <string.h>
LV_FONT_DECLARE(pp_font_14);
#define TOOL_HEIGHT 112
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *game_texture, *tools_texture;
static lv_display_t *game_display, *tools_display;
static lv_indev_t *inputs[2];
static lv_obj_t *zoom_label;
static uint32_t game_pixels[360*360], tools_pixels[360*TOOL_HEIGHT];
static uint16_t game_draw[360*20], tools_draw[360*20];
static SDL_Rect circle, tools;
static int pointer_x, pointer_y, pointer_route;
static bool pressed, dirty, running, suspended;
static bool finger_active;
static SDL_FingerID active_finger;
static unsigned zoom = 90;
static void (*action)(unsigned);
static bool create_textures(void) {
    if(game_texture)SDL_DestroyTexture(game_texture);
    if(tools_texture)SDL_DestroyTexture(tools_texture);
    game_texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,360,360);
    tools_texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,360,TOOL_HEIGHT);
    if(!game_texture || !tools_texture)return false;
    SDL_SetTextureBlendMode(game_texture,SDL_BLENDMODE_BLEND);
    dirty=true;return true;
}

static void layout(void) {
    int w,h;SDL_GetWindowSize(window,&w,&h);
    float fit=(w-24)/360.0f, vertical=(h-24)/488.0f;
    if(vertical<fit)fit=vertical;
    if(fit<0.1f)fit=0.1f;
    int width=(int)(360*fit), height=(int)(TOOL_HEIGHT*fit);
    int top=(h-(int)(488*fit))/2;
    int diameter=width*(int)zoom/100;
    circle=(SDL_Rect){(w-diameter)/2,top+(width-diameter)/2,diameter,diameter};
    tools=(SDL_Rect){(w-width)/2,top+width+(int)(16*fit),width,height};
    SDL_RenderSetLogicalSize(renderer,w,h);
    dirty=true;
}
static void set_zoom(int value) {
    if(value<50)value=50;
    if(value>100)value=100;
    zoom=(unsigned)value;
    lv_label_set_text_fmt(zoom_label,"%u%%",zoom);
    layout();
}
static void button(lv_event_t *e) {
    unsigned id=(unsigned)(uintptr_t)lv_event_get_user_data(e);
    if(id<4)action(id);
    else if(id==4)set_zoom((int)zoom-10);
    else if(id==5)set_zoom((int)zoom+10);
    else set_zoom(100);
    lv_display_set_default(game_display);
}
static void add_button(lv_obj_t *parent,const char *name,int x,int y,int w,unsigned id) {
    lv_obj_t *b=lv_button_create(parent);
    lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,44);
    lv_obj_set_style_radius(b,12,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x233d42),0);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x3b7669),LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(b,0,0);lv_obj_set_style_border_width(b,0,0);
    lv_obj_set_style_text_font(b,&pp_font_14,0);
    lv_obj_add_event_cb(b,button,LV_EVENT_CLICKED,(void *)(uintptr_t)id);
    lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,name);lv_obj_center(l);
    if(id==6)zoom_label=l;
}
static void flush(lv_display_t *display,const lv_area_t *area,uint8_t *pixels) {
    bool game=display==game_display;
    uint32_t *destination=game?game_pixels:tools_pixels;
    unsigned at=0;
    for(int y=area->y1;y<=area->y2;y++)for(int x=area->x1;x<=area->x2;x++) {
        uint16_t color;memcpy(&color,pixels+2*at++,2);
        unsigned r=(color>>11)&31,g=(color>>5)&63,b=color&31;
        uint32_t argb=0xff000000u | ((r*255/31)<<16) | ((g*255/63)<<8) | (b*255/31);
        int dx=2*x-359,dy=2*y-359;
        if(game && dx*dx+dy*dy>360*360)argb=0;
        destination[y*360+x]=argb;
    }
    dirty=true;lv_display_flush_ready(display);
}
bool pp_shell_map(int x,int y,int *gx,int *gy) {
    if(circle.w<=0)return false;
    *gx=(x-circle.x)*360/circle.w;*gy=(y-circle.y)*360/circle.h;
    if(x<circle.x || y<circle.y || x>=circle.x+circle.w || y>=circle.y+circle.h)return false;
    int dx=2*(*gx)-359,dy=2*(*gy)-359;
    return dx*dx+dy*dy<=360*360;
}
static void point(int x,int y,bool down,bool begin) {
    int gx=0,gy=0;
    if(begin) {
        if(pressed && pointer_route) {
            pressed=false;lv_indev_read(inputs[pointer_route-1]);
        }
        pointer_route=0;
        if(pp_shell_map(x,y,&gx,&gy))pointer_route=1;
        else if(x>=tools.x && y>=tools.y && x<tools.x+tools.w && y<tools.y+tools.h)pointer_route=2;
    }
    if(pointer_route==1) {pointer_x=(x-circle.x)*360/circle.w;pointer_y=(y-circle.y)*360/circle.h;}
    else if(pointer_route==2) {pointer_x=(x-tools.x)*360/tools.w;pointer_y=(y-tools.y)*TOOL_HEIGHT/tools.h;}
    pressed=down;
    /* Deliver both edges even when a quick tap arrives between LVGL polls. */
    if(pointer_route)lv_indev_read(inputs[pointer_route-1]);
}
static void read_pointer(lv_indev_t *in,lv_indev_data_t *data) {
    unsigned route=(unsigned)(uintptr_t)lv_indev_get_user_data(in);
    data->state=pressed && pointer_route==(int)route?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
    if(pointer_route==(int)route) {data->point.x=pointer_x;data->point.y=pointer_y;}
}
static lv_display_t *display_create(int height,uint16_t *buffer) {
    lv_display_t *d=lv_display_create(360,height);
    lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(d,buffer,NULL,sizeof(game_draw),LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d,flush);
    return d;
}
bool pp_shell_create(void (*shortcut)(unsigned)) {
    action=shortcut;zoom=90;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS))return false;
    Uint32 flags=SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef __ANDROID__
    flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
    window=SDL_CreateWindow("Pokemon Plus",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,440,600,flags);
    if(!window)return false;
#ifndef __ANDROID__
    SDL_SetWindowMinimumSize(window,300,420);
#endif
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    if(!renderer)renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
    if(!renderer)return false;
    if(!create_textures())return false;
    lv_tick_set_cb(SDL_GetTicks);lv_delay_set_cb(SDL_Delay);
    tools_display=display_create(TOOL_HEIGHT,tools_draw);
    lv_obj_t *screen=lv_display_get_screen_active(tools_display);
    lv_obj_set_style_bg_color(screen,lv_color_hex(0x101c22),0);
    lv_obj_set_style_text_color(screen,lv_color_hex(0xe7f6e9),0);
    lv_obj_set_style_text_font(screen,&pp_font_14,0);
    lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
    const char *names[]={"+100步","摇一摇","+1小时","回庭院"};
    for(unsigned i=0;i<4;i++)add_button(screen,names[i],i*92,0,84,i);
    add_button(screen,"-",68,60,52,4);add_button(screen,"+",240,60,52,5);
    add_button(screen,"90%",132,60,96,6);
    game_display=display_create(360,game_draw);
    for(unsigned route=1;route<=2;route++) {
        lv_indev_t *in=lv_indev_create();lv_indev_set_type(in,LV_INDEV_TYPE_POINTER);
        inputs[route-1]=in;
        lv_indev_set_display(in,route==1?game_display:tools_display);
        lv_indev_set_user_data(in,(void *)(uintptr_t)route);lv_indev_set_read_cb(in,read_pointer);
        lv_indev_set_long_press_time(in,1000);
    }
    lv_display_set_default(game_display);
    layout();running=true;suspended=false;
    return true;
}
bool pp_shell_poll(void) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type==SDL_QUIT)running=false;
        else if(e.type==SDL_RENDER_DEVICE_RESET) {if(!create_textures())running=false;}
        else if(e.type==SDL_RENDER_TARGETS_RESET)dirty=true;
        else if(e.type==SDL_WINDOWEVENT) {
            if(e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event==SDL_WINDOWEVENT_EXPOSED)layout();
            if(e.window.event==SDL_WINDOWEVENT_FOCUS_LOST)pressed=false;
            if(e.window.event==SDL_WINDOWEVENT_FOCUS_GAINED || e.window.event==SDL_WINDOWEVENT_RESTORED) {suspended=false;dirty=true;}
        } else if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT)point(e.button.x,e.button.y,true,true);
        else if(e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT)point(e.button.x,e.button.y,false,false);
        else if(e.type==SDL_MOUSEMOTION && pressed)point(e.motion.x,e.motion.y,true,false);
        else if(e.type==SDL_FINGERDOWN || e.type==SDL_FINGERMOTION || e.type==SDL_FINGERUP) {
            if(e.type==SDL_FINGERDOWN) {
                if(finger_active)continue;
                finger_active=true;active_finger=e.tfinger.fingerId;
            }
            if(!finger_active || e.tfinger.fingerId!=active_finger)continue;
            int w,h;SDL_GetWindowSize(window,&w,&h);
            point((int)(e.tfinger.x*w),(int)(e.tfinger.y*h),e.type!=SDL_FINGERUP,e.type==SDL_FINGERDOWN);
            if(e.type==SDL_FINGERUP)finger_active=false;
        } else if(e.type==SDL_MOUSEWHEEL)set_zoom((int)zoom+(e.wheel.y>0?10:-10));
        else if(e.type==SDL_KEYDOWN && !e.key.repeat) {
            SDL_Keycode k=e.key.keysym.sym;
            if(k==SDLK_k)action(PP_SHORTCUT_STEPS);
            else if(k==SDLK_s)action(PP_SHORTCUT_SHAKE);
            else if(k==SDLK_SPACE)action(PP_SHORTCUT_HOUR);
            else if(k==SDLK_ESCAPE || k==SDLK_AC_BACK)action(PP_SHORTCUT_HOME);
            else if(k==SDLK_EQUALS || k==SDLK_PLUS)set_zoom((int)zoom+10);
            else if(k==SDLK_MINUS)set_zoom((int)zoom-10);
            else if(k>=SDLK_0 && k<=SDLK_9)action(100+(unsigned)(k-SDLK_0));
        } else if(e.type==SDL_APP_WILLENTERBACKGROUND) {suspended=true;pressed=false;finger_active=false;}
        else if(e.type==SDL_APP_DIDENTERFOREGROUND) {suspended=false;dirty=true;}
    }
    return running;
}
void pp_shell_render(void) {
    if(!dirty || suspended)return;
    SDL_UpdateTexture(game_texture,NULL,game_pixels,360*4);
    SDL_UpdateTexture(tools_texture,NULL,tools_pixels,360*4);
    SDL_SetRenderDrawColor(renderer,0x10,0x1c,0x22,255);SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer,game_texture,NULL,&circle);SDL_RenderCopy(renderer,tools_texture,NULL,&tools);
    SDL_RenderPresent(renderer);dirty=false;
}
void pp_shell_destroy(void) {
    if(game_texture)SDL_DestroyTexture(game_texture);
    if(tools_texture)SDL_DestroyTexture(tools_texture);
    if(renderer)SDL_DestroyRenderer(renderer);
    if(window)SDL_DestroyWindow(window);
    SDL_Quit();
}
void pp_shell_resize(int width,int height) {SDL_SetWindowSize(window,width,height);layout();}
unsigned pp_shell_zoom(void) {return zoom;}
void pp_shell_game_point(int gx,int gy,int *x,int *y) {*x=circle.x+gx*circle.w/360;*y=circle.y+gy*circle.h/360;}
void pp_shell_button_point(unsigned id,int *x,int *y) {
    int bx=id<4?(int)id*92+42:id==4?94:id==5?266:180;
    int by=id<4?22:82;
    *x=tools.x+bx*tools.w/360;*y=tools.y+by*tools.h/TOOL_HEIGHT;
}
bool pp_shell_capture(const char *path) {
    int w,h;SDL_GetRendererOutputSize(renderer,&w,&h);
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);
    if(!surface)return false;
    /* Render the same textures again: some backends discard the back buffer
     * immediately after Present. */
    SDL_SetRenderDrawColor(renderer,0x10,0x1c,0x22,255);SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer,game_texture,NULL,&circle);SDL_RenderCopy(renderer,tools_texture,NULL,&tools);
    bool ok=!SDL_RenderReadPixels(renderer,NULL,surface->format->format,surface->pixels,surface->pitch);
    if(ok)ok=SDL_SaveBMP(surface,path)==0;
    SDL_FreeSurface(surface);return ok;
}
