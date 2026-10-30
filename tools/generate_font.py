from pathlib import Path
import urllib.request,urllib.parse,re,subprocess,hashlib,json
root=Path(__file__).resolve().parents[1]
chars=set()
for folder in ['main','core']:
 for p in (root/folder).glob('*.[ch]'):
  if p.name.startswith('pp_font'):continue
  chars.update(c for c in p.read_text() if ord(c)>127)
symbols=''.join(sorted(chars))
url='https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@500&text='+urllib.parse.quote(''.join(chr(i) for i in range(32,127))+symbols)
css=subprocess.check_output(['curl','--silent','--show-error','--fail','--location','--retry','2','--max-time','60',url]).decode();font_url=re.search(r'url\((.*?)\)',css)[1]
font=root/'assets/NotoSansSC-subset.ttf';subprocess.run(['curl','--fail','--location','--retry','2','--max-time','60',font_url,'--output',str(font)],check=True)
(root/'assets/font-source.json').write_text(json.dumps({'family':'Noto Sans SC','license':'SIL OFL 1.1','url':font_url,'sha256':hashlib.sha256(font.read_bytes()).hexdigest()},indent=2))
(root/'assets/font-characters.txt').write_text(symbols)
for size in [12,14,18]:
 subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(font),'--size',str(size),'--bpp','4','--format','lvgl','--range','0x20-0x7e','--symbols',symbols,'--no-compress','--no-kerning','--lv-font-name',f'pp_font_{size}','--lv-include','lvgl.h','-o',str(root/f'main/pp_font_{size}.c')],check=True)
print('Generated',len(symbols),'characters at 12 / 14 / 18px')

# Larger starter and gym-city headings; keep only these glyphs at this size.
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(font),'--size','22','--bpp','4','--format','lvgl','--symbols','选择伙伴尼比市华蓝枯叶玉虹浅红金黄红莲岛常青','--no-compress','--no-kerning','--lv-font-name','pp_font_title_22','--lv-include','lvgl.h','-o',str(root/'main/pp_font_title_22.c')],check=True)
