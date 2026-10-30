"""Compile the shared, reviewed map ecology into a fixed family mask."""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]

def load_catalog():
    data = json.loads((ROOT/'assets/pet-habitats.json').read_text())
    roster = json.loads((ROOT/'assets/pet-roster.json').read_text())
    bases = {p['nationalId']: p['species'] for p in roster if p['stage'] == 0}
    ordinary = set(bases) - {144, 145, 146, 150, 151}
    assert [s['scene'] for s in data['scenes']] == list(range(12))
    covered = set()
    for scene in data['scenes']:
        ids = scene['baseNationalIds']
        assert ids and len(ids) == len(set(ids)) and set(ids) <= ordinary
        covered.update(ids)
    assert covered == ordinary
    return data

def write_header(data=None):
    data = data or load_catalog()
    roster = json.loads((ROOT/'assets/pet-roster.json').read_text())
    masks = []
    for species in range(79):
        base = next(p['nationalId'] for p in roster if p['species'] == species and not p['stage'])
        masks.append(sum(1 << s['scene'] for s in data['scenes'] if base in s['baseNationalIds']))
    source = '#pragma once\n/* Generated from assets/pet-habitats.json; do not edit. */\n'
    source += 'static const uint16_t pp_habitat_masks[PP_SPECIES] = {\n  '
    source += ', '.join(f'0x{mask:03x}' for mask in masks) + '\n};\n'
    (ROOT/'core/pet_habitats.h').write_text(source)

if __name__ == '__main__':
    write_header()
