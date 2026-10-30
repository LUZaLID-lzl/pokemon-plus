"""Small, scoped shop labels; reuse the existing licensed local font."""
from pathlib import Path
import subprocess

root=Path(__file__).resolve().parents[1]
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(root/'assets/NotoSansSC-subset.ttf'),
 '--size','10','--bpp','4','--format','lvgl','--symbols','0123456789树果肥皂玩具精力饼干宠物蛋孵化药剂咕噜球神华',
 '--no-compress','--no-kerning','--lv-font-name','pp_font_shop_10','--lv-include','lvgl.h',
 '-o',str(root/'main/pp_font_shop_10.c')],check=True)
