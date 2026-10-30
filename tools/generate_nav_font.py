# coding: utf-8
"""Small genuine bold font for the three home navigation captions."""
from pathlib import Path
import urllib.request, urllib.parse, re, subprocess, hashlib, json
root=Path(__file__).resolve().parents[1]
symbols='APPBAGSHOP'
url='https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@700&text='+urllib.parse.quote(symbols)
css=urllib.request.urlopen(url,timeout=40).read().decode();font_url=re.search(r'url\((.*?)\)',css)[1]
font=root/'assets/NotoSansSC-nav-bold.ttf'
font.write_bytes(urllib.request.urlopen(font_url,timeout=60).read())
(root/'assets/nav-font-source.json').write_text(json.dumps({'family':'Noto Sans SC','weight':700,'size':14,'license':'SIL OFL 1.1','url':font_url,'sha256':hashlib.sha256(font.read_bytes()).hexdigest()},indent=2)+'\n')
subprocess.run(['npx','--yes','lv_font_conv@1.5.3','--font',str(font),'--size','14','--bpp','4','--format','lvgl','--symbols',symbols,'--no-compress','--no-kerning','--lv-font-name','pp_font_nav_14','--lv-include','lvgl.h','-o',str(root/'main/pp_font_nav_14.c')],check=True)
