#!/usr/bin/env python3
"""Enable reviewed native trim storage only as part of the complete106 release."""
from pathlib import Path
import json
R=Path(__file__).resolve().parents[1]
q=json.loads((R/'docs/gen1-097-106/visual-review.json').read_text())
assert set(range(97,107))<=set(q['animationReviewed'])
assert len(json.loads((R/'assets/pet-roster.json').read_text()))==106
def edit(file,old,new):
 p=R/file;s=p.read_text();assert old in s,(file,old);p.write_text(s.replace(old,new))
edit('tools/prepare_assets.py','from image_binary import write_bin','from image_binary import write_bin\nfrom asset_pet_canvas import encode as trim_pet_canvas')
edit('tools/prepare_assets.py','    exported.update([target/(name+\'.png\'),target/(name+\'.bin\')])', '''    if kind in ('pets','pets-left'):
        native=target/(name+'.bin');native.write_bytes(trim_pet_canvas(native.read_bytes()))
    exported.update([target/(name+'.png'),target/(name+'.bin')])''')
edit('tools/validate_assets.py',"  magic,cf,flags,w,h,stride,reserved=struct.unpack", "  if p.parent.name in ('pets','pets-left'):\n   from asset_pet_canvas import decode\n   data=decode(data)\n  magic,cf,flags,w,h,stride,reserved=struct.unpack")
edit('main/image_assets.h','#include "lvgl.h"','#include "lvgl.h"\n#define PP_IMAGE_CENTERED_PET LV_IMAGE_FLAGS_USER1\nunsigned pp_image_canvas_width(const lv_image_dsc_t *);\nunsigned pp_image_canvas_height(const lv_image_dsc_t *);\nuint32_t pp_image_indexed_pixel(const lv_image_dsc_t *, unsigned x, unsigned y);')
edit('main/image_assets.c','!h->w || !h->h || h->flags || h->magic', '!h->w || !h->h || (h->flags & ~PP_IMAGE_CENTERED_PET) || h->magic')
edit('main/image_assets.c','  if (h->cf == LV_COLOR_FORMAT_I4', '''  if (h->flags && (h->w>48 || h->h>48 || (h->w%2) || (h->h%2) ||
      (h->cf!=LV_COLOR_FORMAT_I4 && h->cf!=LV_COLOR_FORMAT_I8))) return 0;
  if (h->cf == LV_COLOR_FORMAT_I4''')
edit('main/image_assets.c','    if (!image_bytes(&h) || payload!=image_bytes(&h)) goto invalid;', '''    if (!image_bytes(&h) || payload!=image_bytes(&h)) goto invalid;
    if (h.flags && word(native+sizeof(h))) goto invalid;''')
edit('main/image_assets.c','  fclose(file);\n  strcpy(name, path);', '''  fclose(file);
  if (header.flags && (data[0] || data[1] || data[2] || data[3])) {
    free(data);free(name);return path;
  }
  strcpy(name, path);''')
edit('main/image_assets.c','  lv_image_set_src(object, source);', '''  lv_image_set_src(object, source);
  if (source && lv_image_src_get_type(source)==LV_IMAGE_SRC_VARIABLE &&
      (((const lv_image_dsc_t *)source)->header.flags & PP_IMAGE_CENTERED_PET)) {
    /* Only transparent margins were omitted. Preserve the48px object canvas,
     * baseline and center while LVGL draws native Flash pixels directly. */
    lv_obj_set_size(object,48,48);
    lv_image_set_inner_align(object,LV_IMAGE_ALIGN_CENTER);
  }''')
with (R/'main/image_assets.c').open('a') as out:out.write('''
unsigned pp_image_canvas_width(const lv_image_dsc_t *im) {
  return im->header.flags & PP_IMAGE_CENTERED_PET ? 48u : im->header.w;
}
unsigned pp_image_canvas_height(const lv_image_dsc_t *im) {
  return im->header.flags & PP_IMAGE_CENTERED_PET ? 48u : im->header.h;
}
uint32_t pp_image_indexed_pixel(const lv_image_dsc_t *im,unsigned x,unsigned y) {
  if (!im || (im->header.cf!=LV_COLOR_FORMAT_I4 && im->header.cf!=LV_COLOR_FORMAT_I8)) return 0;
  unsigned dx=(pp_image_canvas_width(im)-im->header.w)/2;
  unsigned dy=(pp_image_canvas_height(im)-im->header.h)/2;
  if (x<dx || y<dy || x-dx>=im->header.w || y-dy>=im->header.h) return 0;
  x-=dx;y-=dy;
  unsigned palette=im->header.cf==LV_COLOR_FORMAT_I4?64:1024;
  const uint8_t *indices=im->data+palette;
  unsigned index=im->header.cf==LV_COLOR_FORMAT_I4
    ? (indices[y*im->header.stride+x/2]>>(x%2?0:4))&15
    : indices[y*im->header.stride+x];
  uint32_t pixel;memcpy(&pixel,im->data+index*4,4);return pixel;
}
''')
p=R/'main/battle_view.c';s=p.read_text();start=s.index('  unsigned palette_bytes=im->header.cf');end=s.index('  /* These descriptors',start)
s=s[:start]+'''  for(unsigned y=0;y<PET_SIZE;y++) for(unsigned x=0;x<PET_SIZE;x++) {
    unsigned sx=(2*x+1)*pp_image_canvas_width(im)/(2*PET_SIZE);
    unsigned sy=(2*y+1)*pp_image_canvas_height(im)/(2*PET_SIZE);
    pet_pixels[side][y*PET_SIZE+x]=pp_image_indexed_pixel(im,sx,sy);
  }
'''+s[end:];p.write_text(s)
p=R/'main/game_ui.c';s=p.read_text();start=s.index('  unsigned palette = im->header.cf');end=s.index('  evo_descriptor =',start)
s=s[:start]+'''  for (unsigned y=0;y<96;y++) for(unsigned x=0;x<96;x++) {
    unsigned sx=x*pp_image_canvas_width(im)/96,sy=y*pp_image_canvas_height(im)/96;
    evo_pixels[y*96+x]=pp_image_indexed_pixel(im,sx,sy);
  }
'''+s[end:];p.write_text(s)
with (R/'preview/CMakeLists.txt').open('a') as out:out.write('''
# Independent original48px PNG checks for centered native Flash pet storage.
add_executable(pp_pet_canvas_test ../tests/test_pet_canvas_sampling.c ../main/image_assets.c)
target_include_directories(pp_pet_canvas_test PRIVATE ../tests/mocks ../main)
target_compile_definitions(pp_pet_canvas_test PRIVATE ESP_PLATFORM PP_ASSET_HOST_TEST)
target_compile_options(pp_pet_canvas_test PRIVATE -Wall -Wextra -Werror)
target_link_libraries(pp_pet_canvas_test lvgl ${SDL2_LIBRARIES} m pthread z)
''')
print('Enabled native transparent border trim;48px logical canvas/Flash pointers retained. Full render/build validation required.')
