"""Build shared collection metadata from the reviewed, stable catalog."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
def load_catalog():
 c=json.loads((ROOT/'assets/collection-catalog.json').read_text())
 assert [x['id'] for x in c['items']]==list(range(586))
 assert [sum(x['quality']==q for x in c['items']) for q in range(6)]==[350,146,58,23,6,3]
 return c

def write_header(c):
 text='#pragma once\n/* Generated from assets/collection-catalog.json; IDs preserve save bits. */\n'
 text+='const pp_collection_entry pp_collectibles[PP_COLLECTION] = {\n'
 text+=''.join('  {"%s", %u, %u},\n'%(x['name'],x['quality'],x['scene'] if x['scene'] is not None else 255) for x in c['items'])+'};\n'
 text+='const pp_quality_entry pp_qualities[PP_QUALITY_COUNT] = {\n'
 text+=''.join('  {"%s", 0x%s, %u, %u},\n'%(q['name'],q['color'],q['count'],q['targetPercent']) for q in c['qualities'])+'};\n'
 (ROOT/'core/collection_catalog.h').write_text(text)

def export_collection(export):
 c=load_catalog();write_header(c)
 for x in (x for x in c['items'] if x['id']!=583):
  p=ROOT/f'assets/collection/{x["id"]:03}.png'
  if not p.is_file():raise ValueError(f'Missing reviewed collection image: {p}')
 from PIL import Image
 for x in (x for x in c['items'] if x['id']!=583):
  image=Image.open(ROOT/f'assets/collection/{x["id"]:03}.png').convert('RGBA')
  export(image,'collection',f'{x["id"]:03}',True)
  export(image.resize((20,20),Image.Resampling.NEAREST),'collection',f'{x["id"]:03}-small',True)
 return {k:c[k] for k in ('version','total','travelSlots','classification','rounding','dropRule','qualities','items')}|dict(imagesIntegrated=c['total'],status='integrated',special=c['items'][583:])
if __name__=='__main__':write_header(load_catalog())
