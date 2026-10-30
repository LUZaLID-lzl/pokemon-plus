#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$root/preview" -B "$root/preview/build"
cmake --build "$root/preview/build" -j 8
exec "$root/preview/build/pokemon_plus" --assets "$root/sdcard/pokemon-plus" "$@"
