"""Extract the workbook's read-only terrain reference table."""

import argparse
import json
from pathlib import Path

from openpyxl import load_workbook


ROWS = range(38, 48)
COLUMNS = {
    "terrain": "A",
    "exploration_time": "B",
    "prep_time_cost": "C",
    "farm_road_cost": "D",
}


def extract(workbook: Path) -> dict:
    book = load_workbook(workbook, read_only=True, data_only=False)
    try:
        sheet = book["Turn"]
        entries = []
        for row in ROWS:
            entry = {name: sheet[f"{column}{row}"].value for name, column in COLUMNS.items()}
            entry["source_row"] = row
            entries.append(entry)
        return {
            "source_sheet": "Turn",
            "source_range": "A37:D47",
            "entries": entries,
            "mechanics": "reference_only",
        }
    finally:
        book.close()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("workbook", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    result = extract(args.workbook)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"TERRAIN_REFERENCE_PASS entries={len(result['entries'])} output={args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
