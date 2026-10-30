#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python3 "$root/tools/verify_vendor.py"
idf.py -C "$root" build
python3 "$root/tools/package_firmware.py"
python3 "$root/tools/validate_builtin_firmware.py"
