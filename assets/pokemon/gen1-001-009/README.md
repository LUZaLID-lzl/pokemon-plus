# Gen 1 / 001–009

48×48 transparent PNG, one native pixel grid, <=16 opaque colors.
Each @8x.png is an exact 384×384 nearest-neighbor enlargement.
preview.png is a labeled contact sheet; the individual sprites have no background or labels.

001 Bulbasaur -> 002 Ivysaur -> 003 Venusaur
004 Charmander -> 005 Charmeleon -> 006 Charizard
007 Squirtle -> 008 Wartortle -> 009 Blastoise

MobiIoT generated source atlas on its first attempt. Native sprites were split
from measured source boxes, reduced to a uniform 48px grid, quantized, and had
their silhouette boundaries repaired into connected dark navy outlines.
The 48px option preserves eyes, wings, flower and cannon details.

Source atlas: assets/source/gen1-001-009-v9.png
Prompt: assets/prompts/gen1-001-009-v9.txt
Exporter: tools/export_gen1_001_009.py
References and evolution order: manifest.json

Standalone art review assets: one pose per species. Not yet integrated into the
game's species model, animation atlas or evolution logic.
