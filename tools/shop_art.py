"""Pre-sized shop sprites from the existing item art and MobiIoT lottery master."""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ITEMS = {0:'icons/0.png', 1:'icons/1.png', 3:'icons/3.png',
         4:'icons/4.png', 5:'icons/5.png', 6:'items/hatch-boost.png',
         7:'items/capture-ball.png', 8:'items/tree-essence.png'}

def export_shop(export, root=ROOT):
    runtime = root/'sdcard/pokemon-plus'
    for item, source in ITEMS.items():
        icon = Image.open(runtime/source).convert('RGBA').resize((40,40), Image.Resampling.NEAREST)
        export(icon, 'shop', str(item), True, indexed=False)
    master = Image.open(root/'assets/source/shop-lottery-v2.png').convert('RGBA')
    master.putdata([(0,0,0,0) if a < 200 or (r>180 and b>180 and g<100)
                    else (r,g,b,255) for r,g,b,a in master.getdata()])
    icon = master.crop(master.getbbox())
    icon.thumbnail((48,40), Image.Resampling.NEAREST)
    icon = icon.quantize(colors=15, method=Image.Quantize.FASTOCTREE).convert('RGBA')
    icon.putdata([(r,g,b,255) if a>=200 else (0,0,0,0) for r,g,b,a in icon.getdata()])
    canvas = Image.new('RGBA',(52,44))
    canvas.alpha_composite(icon,((52-icon.width)//2,(44-icon.height)//2))
    export(canvas,'shop','lottery',True,indexed=True)
    coin = Image.open(runtime/'icons/15.png').convert('RGBA').resize((12,12),Image.Resampling.NEAREST)
    export(coin,'shop','coin',True,indexed=True)
