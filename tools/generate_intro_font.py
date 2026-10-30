"""Generate only the opening's bold title fonts; preserve other UI fonts."""
from pathlib import Path
import hashlib,json,re,subprocess,urllib.parse
root=Path(__file__).resolve().parents[1]
symbols='POKEMON PLUS一起开始冒险吧'
font=root/'assets/NotoSansSC-intro-bold.ttf'
source=root/'assets/intro-font-source.json'
if not font.exists():
 url='https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@800&text='+urllib.parse.quote(symbols)
 css=subprocess.check_output(['curl','--silent','--show-error','--fail','--location','--retry','2','--max-time','30',url]).decode()
 font_url=re.search(r'url\((.*?)\)',css)[1]
 subprocess.run(['curl','--silent','--show-error','--fail','--location','--retry','2','--max-time','30',font_url,'--output',str(font)],check=True)
 source.write_text(json.dumps({'family':'Noto Sans SC','weight':800,'symbols':symbols,'license':'SIL OFL 1.1','url':font_url,'sha256':hashlib.sha256(font.read_bytes()).hexdigest()},ensure_ascii=False,indent=2)+'\n')
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(font),'--size','24','--bpp','4','--format','lvgl','--symbols',symbols,'--no-compress','--no-kerning','--lv-font-name','pp_font_intro_24','--lv-include','lvgl.h','-o',str(root/'main/pp_font_intro_24.c')],check=True)

# Slightly larger first title line; use the same cached 800-weight source.
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(font),'--size','32','--bpp','4','--format','lvgl','--symbols','POKEMON','--no-compress','--no-kerning','--lv-font-name','pp_font_intro_32','--lv-include','lvgl.h','-o',str(root/'main/pp_font_intro_32.c')],check=True)

# Smaller welcome line, with the same bold weight.
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(font),'--size','20','--bpp','4','--format','lvgl','--symbols','一起开始冒险吧','--no-compress','--no-kerning','--lv-font-name','pp_font_intro_20','--lv-include','lvgl.h','-o',str(root/'main/pp_font_intro_20.c')],check=True)
