import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
if len(sys.argv) == 3:
    EXECUTABLE = Path(sys.argv[1])
    CATALOG = Path(sys.argv[2])
else:
    executable_candidates = [
        PROJECT_ROOT / "build" / "release-0.1.12-rc" / "Release" / "pkkm.exe",
        PROJECT_ROOT / "build" / "ci" / "Release" / "pkkm.exe",
        PROJECT_ROOT / "build" / "asset-copy-verify" / "Release" / "pkkm.exe",
        PROJECT_ROOT / "build" / "Release" / "pkkm.exe",
    ]
    EXECUTABLE = next((path for path in executable_candidates if path.exists()), executable_candidates[0])
    CATALOG = PROJECT_ROOT / "assets" / "building_catalog.json"
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
        }, {
            "name": "Empty Settlement",
            "population": 250,
            "districts": 1,
            "economy": 0,
            "loyalty": 0,
            "stability": 0,
            "defense": 0,
            "building_inventory": [],
            "apply_catalog_stat_effects": False,
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

    def test_cli_reports_project_version(self):
        result = subprocess.run(
            [str(EXECUTABLE), "--version"],
            check=True,
            capture_output=True,
            text=True,
        )
        self.assertEqual(result.stdout.strip(), "PKKM 0.1.12")
        alias_result = subprocess.run(
            [str(EXECUTABLE), "version"],
            check=True,
            capture_output=True,
            text=True,
        )
        self.assertEqual(alias_result.stdout.strip(), "PKKM 0.1.12")
        version_flag = subprocess.run(
            [str(EXECUTABLE), "--version"],
            check=True,
            capture_output=True,
            text=True,
        )
        self.assertEqual(alias_result.stdout, version_flag.stdout)
        help_result = subprocess.run(
            [str(EXECUTABLE), "--help"],
            check=True,
            capture_output=True,
            text=True,
        )
        self.assertIn("Usage: pkkm --help | --version", help_result.stdout)
        help_alias = subprocess.run(
            [str(EXECUTABLE), "help"],
            check=True,
            capture_output=True,
            text=True,
        )
        self.assertEqual(help_alias.stdout, help_result.stdout)
        invalid_result = subprocess.run(
            [str(EXECUTABLE), "unknown-command"],
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(invalid_result.returncode, 2)
        self.assertIn("Usage: pkkm --help | --version", invalid_result.stderr)


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
        self.assertIn("Audit records | Turn actions 0", output)
        self.assertIn("Economy +4 | Loyalty +4", output)
        self.assertIn("Improvements 0", output)
        self.assertIn("population 1000", output)
        self.assertIn("market base", output)
        self.assertIn("catalog cost", output)
        self.assertIn("Other previews: Corruption", output)
        self.assertIn("Empty Settlement | population 250 | districts 1 | lots 0", output)

    def test_opted_in_catalog_stat_effects_are_applied_to_kingdom_totals(self):
        output = self.run_settlement_preview(True)
        self.assertIn("catalog stat bonuses applied to kingdom totals", output)
        self.assertIn("Economy +4 | Loyalty +4 | Stability 0", output)
        self.assertIn("Defense/Unrest preview only", output)
        self.assertIn("Improvements +4", output)

    def test_random_magic_item_preview_is_seeded_and_non_mutating(self):
        kingdom = json.loads((PROJECT_ROOT / "examples" / "starter.json").read_text(encoding="utf-8"))
        kingdom["schema_version"] = 6
        kingdom["settlements"] = [{
            "name": "Random Item Settlement",
            "population": 0,
            "districts": 0,
            "economy": 0,
            "loyalty": 0,
            "stability": 0,
            "defense": 0,
            "building_inventory": [{"name": "Black Market (2H)", "count": 2}],
            "apply_catalog_stat_effects": False,
            "map": {"rows": 0, "columns": 0, "placements": []},
        }]
        with tempfile.TemporaryDirectory() as directory:
            save_path = Path(directory) / "random-items.json"
            save_path.write_text(json.dumps(kingdom), encoding="utf-8")
            before = save_path.read_bytes()
            result = subprocess.run(
                [str(EXECUTABLE), "random-items", str(save_path), "42"],
                check=True,
                capture_output=True,
                text=True,
            )
            self.assertEqual(save_path.read_bytes(), before)
        self.assertIn("Random Item Settlement | seed 42 | source profile d20pfsrd-all-captured | catalog records 1014 | slots minor 4 medium 2 major 2", result.stdout)
        self.assertEqual(result.stdout.count("random item |"), 8)
        self.assertIn("category roll", result.stdout)
        self.assertIn("variant ", result.stdout)
        self.assertIn("lesser/greater variant selected 50/50", result.stdout)

    def test_random_magic_item_upkeep_populates_and_persists_stock(self):
        kingdom = json.loads((PROJECT_ROOT / "examples" / "starter.json").read_text(encoding="utf-8"))
        kingdom["schema_version"] = 6
        kingdom["settlements"] = [{
            "name": "Upkeep Settlement",
            "population": 0,
            "districts": 0,
            "economy": 0,
            "loyalty": 0,
            "stability": 0,
            "defense": 0,
            "building_inventory": [{"name": "Black Market (2H)", "count": 2}],
            "apply_catalog_stat_effects": False,
            "map": {"rows": 0, "columns": 0, "placements": []},
        }]
        with tempfile.TemporaryDirectory() as directory:
            save_path = Path(directory) / "upkeep.json"
            save_path.write_text(json.dumps(kingdom), encoding="utf-8")
            applied = subprocess.run(
                [str(EXECUTABLE), "random-items-apply", str(save_path), "42"],
                check=True,
                capture_output=True,
                text=True,
            )
            self.assertIn("Applied random magic-item upkeep | seed 42 | populated 8 | catalog records 1014", applied.stdout)
            persisted = json.loads(save_path.read_text(encoding="utf-8"))
            stock = persisted["settlements"][0]["random_magic_item_stock"]
            self.assertEqual(len(stock), 8)
            self.assertTrue(all(entry["state"] in {"generated", "manual_resolution"} for entry in stock))
            repeated = subprocess.run(
                [str(EXECUTABLE), "random-items-apply", str(save_path), "99"],
                check=True,
                capture_output=True,
                text=True,
            )
            self.assertIn("populated 0", repeated.stdout)


if __name__ == "__main__":
    unittest.main()
