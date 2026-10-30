#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$root/preview" -B "$root/preview/build"
cmake --build "$root/preview/build" --target pp_image_bank_test -j8
"$root/preview/build/pp_image_bank_test" "$root/build/assets.bin"
"$root/preview/build/pp_image_bank_test" "$root/build/assets.bin" mismatch
