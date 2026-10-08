import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

if len(sys.argv) != 3:
    raise SystemExit("Usage: test_catalog_cli.py <pkkm.exe> <building_catalog.json>")
EXECUTABLE = Path(sys.argv[1])
CATALOG = Path(sys.argv[2])
PROJECT_ROOT = Path(__file__).resolve().parents[1]
sys.argv = [sys.argv[0]]


class CatalogCliTests(unittest.TestCase):
    def run_settlement_preview(self, apply_effects):
        kingdom = json.loads((PROJECT_ROOT / "examples" / "starter.json").read_text(encoding="utf-8"))
        kingdom["schema_version"] = 6
        kingdom["settlements"] = [{
            "name": "CLI Preview",
            "population": 0,
            "districts": 0,
            "economy": 0,
            "loyalty": 0,
            "stability": 0,
            "defense": 0,
            "building_inventory": [{"name": "Academy", "count": 2}],
            "apply_catalog_stat_effects": apply_effects,
            "map": {"rows": 0, "columns": 0, "placements": []},
        }]
        with tempfile.TemporaryDirectory() as directory:
            save_path = Path(directory) / "effects.json"
            save_path.write_text(json.dumps(kingdom), encoding="utf-8")
            result = subprocess.run(
                [str(EXECUTABLE), "show", str(save_path)],
                check=True,
                capture_output=True,
                text=True,
            )
        return result.stdout

    def test_catalog_emits_one_physical_line_per_improvement(self):
        result = subprocess.run(
            [str(EXECUTABLE), "catalog", str(CATALOG)],
            check=True,
            capture_output=True,
            text=True,
        )
        lines = result.stdout.splitlines()
        self.assertEqual(len(lines), 70)
        university = [line for line in lines if line.startswith("University | Data row 67 |")]
        self.assertEqual(len(university), 1)
        self.assertIn("4 minor scrolls or wondrous items", university[0])

    def test_catalog_stat_effects_are_preview_only_by_default(self):
        output = self.run_settlement_preview(False)
        self.assertIn("catalog effects preview (not applied)", output)
        self.assertIn("Economy +4 | Loyalty +4", output)
        self.assertIn("Improvements 0", output)

    def test_opted_in_catalog_stat_effects_are_applied_to_kingdom_totals(self):
        output = self.run_settlement_preview(True)
        self.assertIn("catalog stat bonuses applied to kingdom totals", output)
        self.assertIn("Economy +4 | Loyalty +4 | Stability 0", output)
        self.assertIn("Defense/Unrest preview only", output)
        self.assertIn("Improvements +4", output)


if __name__ == "__main__":
    unittest.main()
