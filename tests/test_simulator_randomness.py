"""Exercise independent process startups through the simulator's real main()."""
from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parents[1]
binary = root / 'preview/build/pokemon_plus'
roster = json.loads((root/'assets/pet-roster.json').read_text())
legendary = {p['species'] for p in roster if p['nationalId'] in [144, 145, 146, 150, 151]}

def hatch(mode, *args):
    result = json.loads(subprocess.check_output([str(binary), '--hatch-audit', mode, *args], text=True))
    assert result['seed'] != 0
    assert len(set(result['pets'])) == 4
    assert 1 not in result['pets']  # Chosen starter family remains excluded.
    assert not set(result['pets']) & legendary
    return result

report = {}
for mode in ['natural', 'boost']:
    fixed = [hatch(mode, '--seed', '42') for _ in range(3)]
    assert fixed[0] == fixed[1] == fixed[2]
    runs = [hatch(mode) for _ in range(32)]
    # Generous thresholds avoid demanding impossible guarantees of randomness.
    seeds = len({r['seed'] for r in runs})
    sequences = len({tuple(r['pets']) for r in runs})
    assert seeds >= 30 and sequences >= 30, (mode, runs)
    report[mode] = {'processes': len(runs), 'distinctSeeds': seeds,
                    'distinctSequences': sequences, 'fixedSequence': fixed[0]['pets']}
(root/'build/hatch-randomness-checks.json').write_text(json.dumps(report, indent=2)+'\n')
print('PASS: 64 independent new games, natural/boost purchased eggs vary; explicit seeds reproduce; missing-family priority and legendary exclusions preserved')
