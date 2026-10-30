#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$root/preview" -B "$root/preview/build"
cmake --build "$root/preview/build" --target pokemon_plus -j8
mkdir -p "$root/tests/build"
"$root/preview/build/pokemon_plus" --assets "$root/sdcard/pokemon-plus" --scene-damage-smoke
"$root/preview/build/pokemon_plus" --assets "$root/sdcard/pokemon-plus" --scene-profile "$root/tests/build/scene-render-profile.json"
python3 - "$root/tests/build/scene-render-profile.json" <<'PY'
import json,sys
report=json.load(open(sys.argv[1]));samples=report['samples']
assert len(samples)==26
for scene in range(13):
    quiet,active=[p for p in samples if p['scene']==scene]
    assert not quiet['effects'] and active['effects'] and active['frames']>0
    # Allow scenery's existing large updates; particles must not force a
    # repeated whole-screen fallback or transfer half the panel each frame.
    assert active['fullFrames']<=quiet['fullFrames']+2,(scene,active)
    assert active['pixels']<active['frames']*65000,(scene,active)
print('PASS: all13 scene effect render budgets; host timings are informational only')
PY
