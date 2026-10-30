"""Assemble the three moving Blender shots, restrained titles and original audio."""
from pathlib import Path
import json
import math
import subprocess
import wave
import numpy as np
from PIL import Image,ImageDraw,ImageFont

ROOT=Path(__file__).resolve().parents[1]
BUILD=ROOT/'build/lifestyle-v03328'
OUT=ROOT/'docs/videos/lifestyle-v03328'
OUT.mkdir(exist_ok=True,parents=True)
frames=BUILD/'final-frames';frames.mkdir(exist_ok=True)
font='/System/Library/Fonts/Hiragino Sans GB.ttc'
cn=ImageFont.truetype(font,40)
small=ImageFont.truetype(font,18)
latin=ImageFont.truetype('/System/Library/Fonts/Supplemental/Avenir Next.ttc',17) if Path('/System/Library/Fonts/Supplemental/Avenir Next.ttc').exists() else ImageFont.truetype(font,17)
titles=[('场景陪伴','晨间书桌'),('道馆对战','午后咖啡桌'),('宠物仓库','入夜阅读角')]
for shot,kind in enumerate(['scene','battle','warehouse']):
    files=sorted((BUILD/('render-'+kind)).glob('frame_*.png'))
    assert len(files)==120,(kind,len(files))
    for i,file in enumerate(files):
        im=Image.open(file).convert('RGBA')
        title=Image.new('RGBA',im.size);d=ImageDraw.Draw(title)
        fg=(35,54,49,255) if shot<2 else (237,232,217,255)
        sub=(62,78,70,255) if shot<2 else (203,211,215,255)
        d.text((58,114),'POKEMON PLUS',font=latin,fill=sub)
        d.text((56,152),titles[shot][0],font=cn,fill=fg)
        d.line((59,217,99,217),fill=fg,width=2)
        d.text((58,234),titles[shot][1],font=small,fill=sub)
        # Three chapter indicators sit clear of the device and its display.
        for j in range(3):
            x=59+j*33
            d.rounded_rectangle((x,285,x+(19 if j==shot else 7),289),radius=2,fill=fg if j==shot else (*sub[:3],95))
        d.text((58,1190),'游戏运行画面 / 三维场景渲染',font=ImageFont.truetype(font,14),fill=(*sub[:3],185))
        opacity=min(1,(i+1)/12,(120-i)/10)
        title.putalpha(title.getchannel('A').point(lambda a:round(a*opacity)))
        Image.alpha_composite(im,title).convert('RGB').save(frames/f'{shot*120+i:04d}.png')

# A quiet original pentatonic score, generated here with no sampled music.
rate=48000;duration=15
t=np.arange(rate*duration)/rate
audio=np.zeros((len(t),2),dtype=np.float64)
def tone(start,midi,length,gain,pan=.5):
    n=min(int(length*rate),len(t)-int(start*rate));u=np.arange(n)/rate
    hz=440*2**((midi-69)/12)
    env=(1-np.exp(-u*70))*np.exp(-u/1.0)*np.minimum(1,(length-u)/.2)
    signal=(np.sin(2*np.pi*hz*u)+.18*np.sin(2*np.pi*hz*2*u)+.04*np.sin(2*np.pi*hz*3*u))*env*gain
    at=int(start*rate)
    audio[at:at+n,0]+=signal*math.sqrt(1-pan)
    audio[at:at+n,1]+=signal*math.sqrt(pan)
notes=[69,73,76,78,76,73,71,76,74,73,69,71]
for i,n in enumerate(notes):tone(.25+i*1.2,n,2.0,.075,.35+(i%3)*.15)
for start,n in [(0,45),(5,50),(10,47)]:
    tone(start,n,4.6,.045,.5);tone(start+.08,n+7,4.5,.022,.6)
for start in [5,10]:tone(start,88,.6,.035,.5)
fade=np.minimum(1,t/.35)*np.minimum(1,(duration-t)/.7)
audio=np.clip(audio*fade[:,None],-.95,.95)
with wave.open(str(BUILD/'original-score.wav'),'wb') as w:
    w.setnchannels(2);w.setsampwidth(2);w.setframerate(rate);w.writeframes((audio*32767).astype('<i2').tobytes())
video=OUT/'pokemon-plus-life-9x16.mp4'
subprocess.run(['/opt/homebrew/bin/ffmpeg','-hide_banner','-loglevel','warning','-y',
    '-framerate','24','-i',str(frames/'%04d.png'),'-i',str(BUILD/'original-score.wav'),
    '-vf','fade=t=in:st=0:d=0.25,fade=t=out:st=14.65:d=0.35',
    '-c:v','libx264','-preset','slow','-crf','17','-pix_fmt','yuv420p',
    '-c:a','aac','-b:a','160k','-t','15','-movflags','+faststart',str(video)],check=True)
Image.open(frames/'0050.png').save(OUT/'poster.jpg',quality=94)
contact=Image.new('RGB',(1080,640))
for j,index in enumerate([50,170,290]):
    im=Image.open(frames/f'{index:04d}.png');im.thumbnail((360,640));contact.paste(im,(j*360,0))
contact.save(OUT/'storyboard.jpg',quality=94)
probe=subprocess.check_output(['/opt/homebrew/bin/ffprobe','-v','error','-show_streams','-show_format','-of','json',str(video)],text=True)
(OUT/'verification.json').write_text(probe)
print(video)
