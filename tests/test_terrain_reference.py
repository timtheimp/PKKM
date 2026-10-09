"""Test extraction of the source workbook terrain reference table."""

import tempfile
import unittest
from pathlib import Path

from openpyxl import Workbook

import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from import_terrain_reference import _reject_source_overwrite, extract


class TerrainReferenceTests(unittest.TestCase):
    def test_terrain_importer_rejects_source_as_output(self):
        source = Path("source.xlsx")
        with self.assertRaisesRegex(ValueError, "Refusing to overwrite source workbook"):
            _reject_source_overwrite(source, source)

    def test_raw_reference_rows_are_preserved(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "fixture.xlsx"
            workbook = Workbook()
            sheet = workbook.active
            sheet.title = "Turn"
            sheet["A37"] = "Terrain"
            sheet["B37"] = "Exploration Time"
            sheet["C37"] = "Prep Time / Cost"
            sheet["D37"] = "Farm / Road Cost"
            sheet["A38"] = "Forest"
            sheet["B38"] = "2 days"
            sheet["C38"] = "2 months / 4 BP"
            sheet["D38"] = "— / 2BP"
            workbook.save(path)

            result = extract(path)
            self.assertEqual(result["mechanics"], "reference_only")
            self.assertEqual(len(result["entries"]), 10)
            self.assertEqual(result["entries"][0]["terrain"], "Forest")
            self.assertEqual(result["entries"][0]["farm_road_cost"], "— / 2BP")
            self.assertEqual(result["entries"][0]["source_row"], 38)


if __name__ == "__main__":
    unittest.main()
