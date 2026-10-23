#!/usr/bin/env python3
"""Normalize the preserved d20PFSRD random-item tables for later resolution."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

RANGE_RE = re.compile(r"^\s*(\d{1,3})\s*[—–-]\s*(\d{1,3})\s*$")
TIER_RE = re.compile(r"\b(minor|medium|major)\b", re.IGNORECASE)
VARIANT_RE = re.compile(r"\b(lesser|greater)\b", re.IGNORECASE)
PAGE_RE = re.compile(r"/magic-items/([^/?#]+)", re.IGNORECASE)
PRICE_RE = re.compile(r"([0-9][0-9,]*)\s*gp", re.IGNORECASE)


def normalize(source: dict) -> dict:
    records = []
    for page in source:
        url = page.get("url", "")
        page_match = PAGE_RE.search(url)
        category = page_match.group(1) if page_match else "unknown"
        for table in page.get("tables", []):
            lines = table.get("text", "").splitlines()
            if not lines:
                continue
            title = lines[0].strip().removeprefix("Table:").strip()
            tier_match = TIER_RE.search(title)
            variant_match = VARIANT_RE.search(title)
            if not tier_match or not variant_match:
                continue
            tier = tier_match.group(1).lower()
            variant = variant_match.group(1).lower()
            for line in lines[1:]:
                columns = [part.strip() for part in line.split("\t")]
                if len(columns) < 2:
                    continue
                range_match = RANGE_RE.match(columns[0])
                if not range_match:
                    continue
                low, high = map(int, range_match.groups())
                name = columns[1]
                if not name or name.lower() in {"result", "item", "weapon", "ring"}:
                    continue
                price_gp = None
                for column in columns[2:]:
                    price_match = PRICE_RE.search(column)
                    if price_match:
                        price_gp = int(price_match.group(1).replace(",", ""))
                        break
                    if column.replace(",", "").isdigit():
                        price_gp = int(column.replace(",", ""))
                        break
                records.append(
                    {
                        "category": category,
                        "tier": tier,
                        "variant": variant,
                        "table": title,
                        "low": low,
                        "high": high,
                        "name": name,
                        "price_gp": price_gp,
                        "source_url": url,
                    }
                )
    return {
        "source_profile": "d20pfsrd-all-captured",
        "record_count": len(records),
        "records": records,
    }


def resolve(catalog: dict, category: str, tier: str, variant: str, roll: int) -> dict | None:
    """Resolve a d100 roll against one normalized category table."""
    if not 1 <= roll <= 100:
        raise ValueError("roll must be between 1 and 100")
    matches = [
        record for record in catalog["records"]
        if record["category"] == category
        and record["tier"] == tier
        and record["variant"] == variant
        and record["low"] <= roll <= record["high"]
    ]
    return matches[0] if matches else None


CATEGORY_SLUGS = {
    "Armor and shields": "magic-armor",
    "Weapons": "magic-weapons",
    "Potions": "potions",
    "Rings": "rings",
    "Rods": "rods",
    "Scrolls": "scrolls",
    "Staves": "staves",
    "Wands": "wands",
    "Wondrous items": "wondrous-items",
}


def resolve_category_result(catalog: dict, category: str, tier: str, variant: str, roll: int) -> dict:
    """Resolve a category while naming source-defined deferred branches."""
    if category not in CATEGORY_SLUGS:
        raise ValueError(f"unsupported random-item category: {category}")
    slug = CATEGORY_SLUGS[category]
    category_records = [
        record for record in catalog["records"]
        if record["category"] == slug
        and record["tier"] == tier
        and record["variant"] == variant
    ]
    if category in {"Potions", "Scrolls", "Wands"}:
        return {
            "status": "manual_spell_selection_required",
            "category": category,
            "tier": tier,
            "variant": variant,
            "roll": roll,
            "available_records": len(category_records),
        }
    matching_tables = sorted({record["table"] for record in category_records})
    if len(matching_tables) != 1:
        return {
            "status": "manual_subtable_selection_required",
            "category": category,
            "tier": tier,
            "variant": variant,
            "roll": roll,
            "available_tables": matching_tables,
        }
    record = resolve(catalog, slug, tier, variant, roll)
    if record is None:
        return {
            "status": "no_source_row_for_roll",
            "category": category,
            "tier": tier,
            "variant": variant,
            "roll": roll,
            "table": matching_tables[0],
        }
    return {"status": "resolved", **record}


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(f"usage: {argv[0]} SOURCE_JSON OUTPUT_JSON", file=sys.stderr)
        return 2
    source_path, output_path = map(Path, argv[1:])
    normalized = normalize(json.loads(source_path.read_text(encoding="utf-8")))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(normalized, indent=2) + "\n", encoding="utf-8")
    print(f"normalized {normalized['record_count']} records")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
