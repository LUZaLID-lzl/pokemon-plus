from pathlib import Path
import urllib.request,urllib.parse,re,subprocess,json,hashlib
r=Path(__file__).resolve().parents[1]
u='https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@900&text='+urllib.parse.quote('0123456789:-')
css=urllib.request.urlopen(u,timeout=30).read().decode();url=re.search(r'url\((.*?)\)',css)[1]
f=r/'assets/clock-digits.ttf';f.write_bytes(urllib.request.urlopen(url,timeout=30).read())
(r/'assets/clock-font-source.json').write_text(json.dumps({'family':'Noto Sans SC','weight':900,'license':'SIL OFL 1.1','source':url,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()},indent=2))
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(f),'--size','36','--bpp','4','--format','lvgl','--symbols','0123456789:-','--no-compress','--no-kerning','--lv-font-name','pp_clock_36','--lv-include','lvgl.h','-o',str(r/'main/pp_clock_36.c')],check=True)


# Home clock is slightly larger than the step counter.
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(f),'--size','38','--bpp','4','--format','lvgl','--symbols','0123456789:-','--no-compress','--no-kerning','--lv-font-name','pp_clock_38','--lv-include','lvgl.h','-o',str(r/'main/pp_clock_38.c')],check=True)
