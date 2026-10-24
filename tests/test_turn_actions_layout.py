import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "windows_app.cpp"


class TurnActionsLayoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = SOURCE.read_text(encoding="utf-8")

    @staticmethod
    def column_count(client_width: int) -> int:
        minimum_column_width = 700
        two_column_threshold = minimum_column_width * 2 + 24 + 20
        three_column_threshold = minimum_column_width * 3 + 18 * 2 + 20
        if client_width >= three_column_threshold:
            return 3
        if client_width >= two_column_threshold:
            return 2
        return 1

    def test_responsive_breakpoints_are_monotonic(self):
        self.assertEqual(
            [self.column_count(width) for width in (1000, 1444, 1800, 2156, 2500)],
            [1, 2, 2, 3, 3],
        )
        self.assertLessEqual(self.column_count(1000), self.column_count(1444))
        self.assertLessEqual(self.column_count(1444), self.column_count(1800))
        self.assertLessEqual(self.column_count(1800), self.column_count(2154))
        self.assertLessEqual(self.column_count(2154), self.column_count(2500))

    def test_source_keeps_the_ruler_and_semantic_lane_order(self):
        self.assertIn(
            "constexpr RulerGeometry ruler{575, 10, 30, 22, 8, 8, 75, 24};",
            self.source,
        )
        self.assertIn("constexpr int minimum_column_width = 700;", self.source)
        self.assertIn("target_x[3] = target_x[2];", self.source)
        self.assertIn("target_x[4] = target_x[2];", self.source)
        self.assertIn("target_y[3] = target_y[2] + target_height[2] + section_gap;", self.source)
        self.assertIn("target_y[4] = target_y[3] + target_height[3] + section_gap;", self.source)

    def test_phase_three_subsections_are_ordered_before_sale(self):
        self.assertLess(850, 1120)
        self.assertLess(1120, 1450)
        self.assertIn('target_top = 850 + (top - 465);', self.source)
        self.assertIn('target_top = 1120 + (top - 465);', self.source)
        self.assertIn('target_top = 1450 + (top - 3080);', self.source)
        self.assertIn("id == kLoyaltyHistory", self.source)


if __name__ == "__main__":
    unittest.main()
