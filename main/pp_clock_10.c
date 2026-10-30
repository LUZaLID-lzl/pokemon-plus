/*******************************************************************************
 * Size: 10 px
 * Bpp: 4
 * Opts: --font assets/clock-digits.ttf --size 10 --bpp 4 --format lvgl --symbols 0123456789- --no-compress --no-kerning --lv-font-name pp_clock_10 --lv-include lvgl.h -o main/pp_clock_10.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef PP_CLOCK_10
#define PP_CLOCK_10 1
#endif

#if PP_CLOCK_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+002D "-" */
    0x48, 0x82, 0x5b, 0xb3,

    /* U+0030 "0" */
    0x1, 0x78, 0x10, 0xd, 0xfe, 0xe1, 0x5f, 0x43,
    0xf7, 0x8f, 0x20, 0xfa, 0x9f, 0x10, 0xfa, 0x7f,
    0x21, 0xf8, 0x3f, 0x98, 0xf4, 0x7, 0xff, 0x90,
    0x0, 0x1, 0x0,

    /* U+0031 "1" */
    0x0, 0x47, 0x10, 0xe, 0xff, 0x20, 0x5, 0xcf,
    0x20, 0x0, 0xaf, 0x20, 0x0, 0xaf, 0x20, 0x0,
    0xaf, 0x20, 0x17, 0xcf, 0x83, 0x3f, 0xff, 0xf7,

    /* U+0032 "2" */
    0x4, 0x87, 0x10, 0x6f, 0xef, 0xd0, 0x4, 0x9,
    0xf3, 0x0, 0xa, 0xf1, 0x0, 0x3f, 0xa0, 0x2,
    0xed, 0x10, 0x2e, 0xf9, 0x74, 0x9f, 0xff, 0xf9,

    /* U+0033 "3" */
    0x4, 0x87, 0x20, 0x4f, 0xef, 0xf1, 0x3, 0x9,
    0xf4, 0x1, 0x7e, 0xd0, 0x2, 0xdf, 0xa0, 0x0,
    0x6, 0xf6, 0x5b, 0x5a, 0xf6, 0x5e, 0xff, 0xa0,
    0x0, 0x10, 0x0,

    /* U+0034 "4" */
    0x0, 0x27, 0x70, 0x0, 0xcf, 0xf0, 0x5, 0xfc,
    0xf0, 0xe, 0xaa, 0xf0, 0x8f, 0x5d, 0xf2, 0xcf,
    0xff, 0xfd, 0x12, 0x2c, 0xf2, 0x0, 0xa, 0xf0,

    /* U+0035 "5" */
    0x7, 0x77, 0x71, 0xf, 0xff, 0xf3, 0x1f, 0x80,
    0x0, 0x1f, 0xec, 0x70, 0x9, 0x7c, 0xf5, 0x0,
    0x4, 0xf8, 0x4a, 0x5b, 0xf5, 0x4e, 0xff, 0x90,
    0x0, 0x10, 0x0,

    /* U+0036 "6" */
    0x0, 0x58, 0x50, 0xa, 0xfe, 0xf4, 0x3f, 0x70,
    0x20, 0x7f, 0x7a, 0x70, 0x8f, 0xda, 0xf7, 0x6f,
    0x30, 0xfa, 0x2f, 0xa5, 0xf8, 0x5, 0xff, 0xb0,
    0x0, 0x1, 0x0,

    /* U+0037 "7" */
    0x37, 0x77, 0x74, 0x7f, 0xff, 0xf9, 0x0, 0x9,
    0xe1, 0x0, 0x2f, 0x60, 0x0, 0x9f, 0x0, 0x0,
    0xec, 0x0, 0x0, 0xfa, 0x0, 0x2, 0xf9, 0x0,

    /* U+0038 "8" */
    0x1, 0x77, 0x20, 0x1e, 0xed, 0xf1, 0x3f, 0x62,
    0xf4, 0xd, 0xea, 0xd0, 0xa, 0xef, 0xc1, 0x5f,
    0x15, 0xf7, 0x7f, 0x54, 0xf8, 0xb, 0xff, 0xc1,
    0x0, 0x1, 0x0,

    /* U+0039 "9" */
    0x3, 0x86, 0x0, 0x3f, 0xef, 0xc0, 0x9f, 0x14,
    0xf5, 0x8f, 0x25, 0xf8, 0x2f, 0xfe, 0xf9, 0x0,
    0x33, 0xf7, 0x9, 0x5c, 0xf1, 0x3e, 0xfe, 0x50,
    0x0, 0x10, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 61, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 4, .adv_w = 97, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 31, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 97, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 106, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 97, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 157, .adv_w = 97, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 184, .adv_w = 97, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 97, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 235, .adv_w = 97, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 0, 0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 45, .range_length = 13, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 13, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t pp_clock_10 = {
#else
lv_font_t pp_clock_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if PP_CLOCK_10*/

