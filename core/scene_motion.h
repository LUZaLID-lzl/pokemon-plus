#pragma once
#include <stdbool.h>
#include <stdint.h>
#define PP_SCENE_NATIVE 120
#define PP_SCENE_PIXELS (PP_SCENE_NATIVE*PP_SCENE_NATIVE)
#define PP_SCENE_MOTION_TICK 120u
/* Input/output are separate native RGB565 canvases, retained by the caller. */
void pp_scene_motion_render(unsigned scene, uint32_t elapsed,
                            const uint16_t *base, uint16_t *output);
bool pp_scene_motion_safe(int x, int y);
