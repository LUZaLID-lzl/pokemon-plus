"""Resume explicitly requested batch; preserve failed attempts and accepted artwork."""
from pathlib import Path
import subprocess,sys,json
R=Path(__file__).resolve().parents[1]
cli=Path.home()/'.codex/skills/mobiiot-imagegen/scripts/imagegen.py'
for dex,version in [(19,2),(25,2),(26,1),(27,1)]:
 folder=R/f'assets/prompts/animation-{dex:03}';folder.mkdir(parents=True,exist_ok=True)
 prompt=folder/f'master-v{version}.txt';out=R/f'assets/source/animation-{dex:03}-master-v{version}.png'
 if out.exists():continue
 if dex in (19,25):
  text=(folder/'master-v1.txt').read_text()
  if dex==19:text+='\nLAYOUT CORRECTION: previous draft had only five rows and omitted sleep. You MUST draw SIX FULL ROWS with eight isolated creatures per row. Row5 must be a complete row of eight low resting CLOSED-EYE mice; row6 must still contain eight attack phases. Count every row before returning. Leave uniform transparent gutters between all six rows.\n'
  prompt.write_text(text)
 elif not prompt.exists():
  # Reuse the common production contract, supplying the exact target identity.
  text=(R/'assets/prompts/animation-025/master-v1.txt').read_text()
  text=text.replace('025 Pikachu',f'{dex:03} '+('Raichu' if dex==26 else 'Sandshrew'))
  start=text.index('Preserve ');end=text.index('. Match reference',start)
  identity=('ORANGE mouse, cream belly, long dark-tipped curled ears, yellow cheeks, long THIN dark attached tail ending in yellow lightning shape' if dex==26 else 'yellow-tan armored pangolin with segmented brick-like shell plates on head and back, cream muzzle and belly, small ears, short attached tail, short claws and four limbs, no back spikes')
  text=text[:start]+'Preserve '+identity+text[end:]
  if dex==27:text=text.replace('Mouse anatomy: eight biped alternating foot walk phases, arms counterbalance, attached lightning tail moves subtly.','Pangolin anatomy: eight quadruped alternating foot walk phases with true joint articulation; keep shell plates stable and small attached tail.').replace('Ears/tail stay attached, no electric effects.','Shell and tail stay attached, no effects.')
  prompt.write_text(text)
 print(f'Generating {dex:03} v{version}',flush=True)
 with (folder/f'generation-v{version}.log').open('w') as log:
  result=subprocess.run([sys.executable,str(cli),'edit','--image',str(R/f'assets/pokemon/gen1-019-027/{dex:03}@8x.png'),'--prompt-file',str(prompt),'--output',str(out)],stdout=log,stderr=subprocess.STDOUT)
 if result.returncode:
  print(f'STOP: {dex:03} failed; see generation-v{version}.log',flush=True);sys.exit(result.returncode)
 print(f'Generated {dex:03} v{version}',flush=True)
