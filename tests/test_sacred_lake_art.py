"""Regression checks against reviewed native art, not just an abstract altar box."""
from pathlib import Path
from PIL import Image
import subprocess
root=Path(__file__).resolve().parents[1]
scene=Image.open(root/'sdcard/pokemon-plus/scenes/12.png').convert('RGB').resize((120,120),Image.Resampling.NEAREST)
front=Image.open(root/'sdcard/pokemon-plus/scene-fronts/12.png').convert('RGBA')
alpha=front.getchannel('A')
# Right S-bend leaves must occlude, while the actual water gaps remain open.
for point in [(84,55),(82,58),(77,50),(71,44),(70,47),(72,49),
              (73,50),(34,78),(56,76),(85,80)]:
    assert alpha.getpixel(point)==255,('missing foreground',point)
for point in [(50,50),(60,56),(68,57),(49,66)]:
    assert alpha.getpixel(point)==0,('water incorrectly occludes',point)
for y in range(120):
    for x in range(120):
        assert alpha.getpixel((x,y)) in (0,255)
        if alpha.getpixel((x,y)):
            assert front.getpixel((x,y))[:3]==scene.getpixel((x,y))
positions=0
for row in subprocess.check_output([str(root/'tests/build/test_sacred_lake'),'--samples'],text=True).splitlines():
    a=row.split()
    x,y=map(int,a[2:4] if a[0]=='P' else a[-2:])
    foot=y+42
    # Test the whole13px-wide foot contact, including the actual flared roots.
    if foot>=204:
        for px in range(x+18,x+31):
            assert not alpha.getpixel((px//3,foot//3)),('feet intersect roots',row,px,foot)
    positions+=1
print(f'PASS: right-bend leaves/branch windows and native foreground colors; {positions} patrol/item foot contacts clear actual roots')
