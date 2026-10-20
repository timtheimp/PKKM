import unittest

from tools.normalize_random_magic_item_tables import normalize, resolve, resolve_category_result


class RandomMagicItemTableTests(unittest.TestCase):
    def test_normalizes_and_resolves_variant_table(self):
        source = [{
            "url": "https://www.d20pfsrd.com/magic-items/rings",
            "tables": [{
                "text": "Greater Minor Rings\nd%\tRing\tPrice (gp)\tSource\n01–15\tRing of counterspells\t4,000\tPRG:UE\n16–100\tRing of grit mastery\t6,840\tPRG:UE"
            }]
        }]
        catalog = normalize(source)
        self.assertEqual(catalog["record_count"], 2)
        result = resolve(catalog, "rings", "minor", "greater", 10)
        self.assertEqual(result["name"], "Ring of counterspells")
        self.assertEqual(result["price_gp"], 4000)

    def test_rejects_invalid_roll(self):
        with self.assertRaises(ValueError):
            resolve({"records": []}, "rings", "minor", "greater", 0)

    def test_names_spell_selection_as_manual(self):
        result = resolve_category_result(
            {"records": []}, "Scrolls", "minor", "lesser", 50
        )
        self.assertEqual(result["status"], "manual_spell_selection_required")

    def test_resolves_direct_category_result(self):
        source = [{
            "url": "https://www.d20pfsrd.com/magic-items/rings",
            "tables": [{
                "text": "Greater Minor Rings\nd%\tRing\tPrice (gp)\tSource\n01–100\tRing of counterspells\t4,000\tPRG:UE"
            }]
        }]
        result = resolve_category_result(
            normalize(source), "Rings", "minor", "greater", 42
        )
        self.assertEqual(result["status"], "resolved")
        self.assertEqual(result["name"], "Ring of counterspells")


if __name__ == "__main__":
    unittest.main()
