import sys
import tempfile
import unittest
from pathlib import Path

from openpyxl import Workbook
from openpyxl.workbook.defined_name import DefinedName

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from import_building_catalog import export_catalog


class BuildingCatalogImportTests(unittest.TestCase):
    def test_import_preserves_non_building_rows_and_maps_improvements(self):
        headers = [
            "Improvements", "Cost", "Lots", "Economy", "Loyalty", "Stability", "Defense", "Unrest",
            "Base Value", "Discount", "Magic Item", "Upgrade From", "Upgrade To", "Corruption", "Crime",
            "Law", "Lore", "Society", "Productivity", "Fame",
        ]
        workbook = Workbook()
        sheet = workbook.active
        sheet.title = "Data"
        sheet.append(headers)
        for label in ("Basic", "Town", "City", "District"):
            sheet.append([label] + [None] * (len(headers) - 1))
        sheet.append([
            "Test Hall", 22, 2, 1, 2, 3, 4, -1, 5, "Discount", "Magic", "Old Hall", "New Hall",
            1, 2, 3, 4, 5, 6, 7,
        ])
        workbook.defined_names.add(DefinedName("IMP", attr_text="'Data'!$A$2:$T$6"))

        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "catalog.xlsx"
            workbook.save(source)
            catalog = export_catalog(source)

        records = catalog["records"]
        self.assertEqual([record["kind"] for record in records], ["basic", "town", "city", "district", "improvement"])
        self.assertEqual([record["source_row"] for record in records], [2, 3, 4, 5, 6])
        self.assertEqual(records[0]["source_values"][0], "Basic")
        self.assertEqual(len(records[0]["source_values"]), 20)
        self.assertEqual(records[4]["name"], "Test Hall")
        self.assertEqual(records[4]["building"], {
            "cost": 22, "lots": 2, "economy": 1, "loyalty": 2, "stability": 3,
            "defense": 4, "unrest": -1, "base_value": 5, "discounts": "Discount",
            "magic_item": "Magic", "upgrade_from": "Old Hall", "upgrade_to": "New Hall",
            "corruption": 1, "crime": 2, "law": 3, "lore": 4, "society": 5,
            "productivity": 6, "fame": 7,
        })


if __name__ == "__main__":
    unittest.main()
