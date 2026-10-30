#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$root/preview" -B "$root/preview/build"
cmake --build "$root/preview/build" --target pokemon_plus -j8
export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy}
export SDL_RENDER_DRIVER=${SDL_RENDER_DRIVER:-software}
for scenario in --completion-smoke --training-growth-smoke --travel-smoke --long-home-smoke; do
  "$root/preview/build/pokemon_plus" --assets "$root/sdcard/pokemon-plus" "$scenario"
done
