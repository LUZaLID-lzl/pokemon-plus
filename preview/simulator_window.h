#ifndef PP_SIMULATOR_WINDOW_H
#define PP_SIMULATOR_WINDOW_H
#include "lvgl.h"
#include <stdbool.h>
enum { PP_SHORTCUT_STEPS, PP_SHORTCUT_SHAKE, PP_SHORTCUT_HOUR, PP_SHORTCUT_HOME };
bool pp_shell_create(void (*shortcut)(unsigned));
bool pp_shell_poll(void);
void pp_shell_render(void);
void pp_shell_destroy(void);
void pp_shell_resize(int width, int height);
bool pp_shell_capture(const char *path);
bool pp_shell_map(int x, int y, int *game_x, int *game_y);
void pp_shell_button_point(unsigned button, int *x, int *y);
void pp_shell_game_point(int game_x, int game_y, int *x, int *y);
unsigned pp_shell_zoom(void);
#endif
