#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$root/preview" -B "$root/preview/build"
cmake --build "$root/preview/build" --target pokemon_plus -j8
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy}
for scene in 0 1 2 3 4 5; do
 for party in 1 2 3; do
  "$root/preview/build/pokemon_plus" --assets "$root/sdcard/pokemon-plus" --training-profile --training-scene "$scene" --training-party "$party"
 done
done
