"""Editable, manually plotted pixel template from the user's close-up.

This is native SVG/pixel drawing, NOT a successful image-service generation.
It is a standalone review candidate and does not replace integrated game art.
"""
from pathlib import Path
from PIL import Image, ImageDraw
import json
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'assets/source'
COLORS={'outline':'#4b4329','tip':'#353b2b','yellow':'#f8df37',
        'light':'#fff176','shine':'#fff8a2','shade':'#d4a62c',
        'deep':'#b17b29','cheek':'#ed4930','cheeklight':'#ff6840'}
im=Image.new('RGBA',(32,32));draw=ImageDraw.Draw(im)
def polygon(points,color):draw.polygon(points,fill=COLORS[color])
def box(bounds,color):draw.rectangle(bounds,fill=COLORS[color])
# Tail is behind the body, a broad squared lightning zigzag on the right.
polygon([(23,7),(30,7),(30,14),(27,14),(27,16),(25,16),(25,19),(23,21),(21,18),(24,15),(26,14),(23,13)],'outline')
polygon([(24,8),(29,8),(29,13),(26,13),(26,15),(24,15),(24,18),(23,19),(22,18),(25,15),(27,14),(24,12)],'yellow')
box((25,8,28,9),'light');box((24,17,24,18),'deep')
# Short upright ears; no strokes/dilation are added around the pixel mask.
polygon([(9,2),(11,2),(11,3),(12,3),(12,7),(13,8),(11,11),(8,10),(8,5),(9,5)],'outline')
polygon([(9,5),(10,5),(10,4),(11,4),(11,7),(12,8),(10,10),(9,9)],'yellow')
box((9,2,11,3),'tip');box((8,5,8,6),'tip');box((9,7,10,8),'light')
polygon([(22,3),(24,3),(24,7),(23,7),(23,9),(21,11),(19,9),(20,7),(21,7),(21,5),(22,5)],'outline')
polygon([(22,6),(23,6),(23,7),(22,7),(22,9),(20,10),(20,8),(21,8),(21,7),(22,7)],'yellow')
box((22,3,24,4),'tip');box((21,8,21,9),'light')
# The head and torso are one compact dumpling silhouette, as in the photo.
polygon([(10,8),(19,8),(19,7),(21,7),(21,9),(23,9),(23,12),(24,12),(24,16),(25,16),(25,21),(24,21),(24,24),(23,24),(23,27),(18,27),(18,26),(11,26),(11,27),(6,27),(6,24),(5,24),(5,16),(6,16),(6,12),(7,12),(7,10),(10,10)],'outline')
polygon([(10,9),(18,9),(18,8),(20,8),(20,10),(22,10),(22,13),(23,13),(23,17),(24,17),(24,20),(23,20),(23,23),(22,23),(22,26),(19,26),(19,25),(10,25),(10,26),(7,26),(7,23),(6,23),(6,17),(7,17),(7,12),(8,12),(8,11),(10,11)],'yellow')
# Minimal warm shadows, no gradients or glossy rendering.
polygon([(22,12),(23,13),(23,17),(24,17),(24,20),(23,20),(23,23),(21,23),(21,24),(17,24),(17,23),(21,23),(21,21),(22,21)],'shade')
box((9,24,20,24),'shade');box((8,25,10,26),'shade');box((19,25,21,26),'shade')
polygon([(10,10),(18,10),(18,11),(20,11),(20,12),(9,12),(9,15),(8,15),(8,18),(7,18),(7,13),(8,13),(8,11),(10,11)],'light')
box((10,10,14,10),'shine');box((9,19,10,21),'light');box((11,19,18,20),'light')
# Small vertical eyes, tiny mouth and square red cheeks.
box((10,13,10,15),'outline');box((11,13,11,14),'outline')
box((19,13,19,15),'outline');box((20,13,20,14),'outline')
box((7,16,9,18),'cheek');box((7,16,8,16),'cheeklight')
box((21,16,23,18),'cheek');box((21,16,22,16),'cheeklight')
box((15,15,15,15),'outline')
for x,y in [(14,17),(16,17),(15,18)]:box((x,y,x,y),'deep')
# Two tiny paws point inward, feet stay very short.
for x,y in [(11,20),(11,21),(10,22),(11,22),(17,20),(17,21),(17,22),(18,22)]:box((x,y,x,y),'deep')
box((12,21,12,22),'light');box((16,21,16,22),'light')
im.save(OUT/'pikachu-reference-template-v6.png')
im.resize((320,320),Image.Resampling.NEAREST).save(OUT/'pikachu-reference-template-v6-10x.png')
preview=Image.new('RGBA',(320,320),'#d0dfd5')
preview.alpha_composite(im.resize((320,320),Image.Resampling.NEAREST))
preview.convert('RGB').save(OUT/'pikachu-reference-template-v6-preview.png')
# RLE each row into native SVG rectangles so the artwork remains editable.
rectangles=[]
for y in range(32):
 x=0
 while x<32:
  color=im.getpixel((x,y));end=x+1
  while end<32 and im.getpixel((end,y))==color:end+=1
  if color[3]:rectangles.append(f'<rect x="{x}" y="{y}" width="{end-x}" height="1" fill="#{color[0]:02x}{color[1]:02x}{color[2]:02x}"/>')
  x=end
svg='<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="320" height="320" shape-rendering="crispEdges">'+''.join(rectangles)+'</svg>\n'
(OUT/'pikachu-reference-template-v6.svg').write_text(svg)
meta={'kind':'manual pixel/SVG drawing','reference':'reference/pikachu-closeup-20261022.png','canvas':[32,32],'colors':COLORS,'integrated':False,'source':'tools/draw_pikachu_reference.py','imageServiceGenerated':False}
(ROOT/'assets/pikachu-reference-template-v6.json').write_text(json.dumps(meta,indent=2))
