# PKKM v0.1.12

## Summary

PKKM v0.1.12 delivers the schema-v10 standalone Kingdom Manager with seeded settlement magic-item workflows, guided turn-phase adapters, source-faithful army and city worksheet support, Greenbelt editing, and responsive native Win32 action surfaces.

## Included

- Schema-v10 runtime packaging with `random_magic_item_catalog.json`.
- Seeded, auditable settlement magic-item preview/application and explicit manual-resolution states for deferred source branches.
- Guided Turn Checklist workflow ordered as Upkeep, Improvement, Income, and Event.
- Guarded checklist adapters and handoffs for claim hexes, withdrawals, deposits, item deposits, taxes, event rolls/resolution, construction, city worksheets, army worksheets, and law inputs.
- Source-faithful army roster and creation worksheet with caller-supplied manual combat fields and explicit BP resource costs.
- Responsive Turn Checklist layout with separated lower bands and reserved Apply/Cancel/Close controls.
- Responsive Greenbelt Map Editor with right-docked tool controls during resize and maximize.
- Responsive Turn Actions layout with titled phase sections, compacted phase transitions, safe one-column scrolling, and maximized multi-column reflow.
- Restored modeless `Turn Rules` access beside `Turn Actions` on the Turn Checklist.
- Existing PDF-first rules profile, manual outcome boundaries, schema-v10 persistence, native Win32 workflows, and deterministic package validation.

## Verification

- Release targets built successfully with Visual Studio 18 2026.
- CTest passed `2/2`.
- Native regression executable passed.
- Python suite passed `21` tests.
- CLI version reports `PKKM 0.1.12`.
- Deterministic package passed exact five-entry validation.
- CPack archive passed exact five-entry validation.
- Fresh native captures verified normal, scrolled, and maximized Turn Actions layouts, including compact Gold Deposit/Sell Valuable Item and Event phase transitions.
- Fresh native capture verified `Turn Rules` beside `Turn Actions` and confirmed the modeless Turn Rules window opens.
- Native control geometry inspection reported zero control overlaps.

Published `v0.1.11` remains unchanged.
