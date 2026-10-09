# PKKM — Kingmaker Kingdom Manager (initial native Windows release)

[![PKKM CI](https://github.com/timtheimp/PKKM/actions/workflows/ci.yml/badge.svg)](https://github.com/timtheimp/PKKM/actions/workflows/ci.yml)

Contribution and workbook-safety guidance is in [`CONTRIBUTING.md`](CONTRIBUTING.md).

PKKM is released under the [MIT License](LICENSE).

## Token-saving workflow

`TOKEN_SAVER.md` records the project-specific staged inspection, testing, GUI, and packaging workflow used to reduce redundant tool calls without weakening verification.

PKKM is a local-first C++17 Win32 desktop application for kingdom management. Its framework-independent domain library calculates the currently implemented stat, alignment, law/edict, leadership, Control DC, population, and settlement aggregation rules. Kingdom data is versioned UTF-8 JSON schema v9; schemas v1-v8 migrate with empty calendar-note history, while schemas v1-v7 also receive empty turn-checklist history and existing settlement/map migration defaults remain unchanged. If settlement records exist, their count and stat totals feed the kingdom summary; population and districts derive from catalog lots and stored overrides, while legacy territory aggregates remain the fallback when there are no settlement records. The CLI and desktop dashboard show signed stat modifier breakdowns and settlement totals. The Win32 editor exposes kingdom, statistic, law, and leadership inputs. **Manage Settlements** edits settlement fields, catalog-backed inventory, and a per-settlement checkbox for applying catalog Economy/Loyalty/Stability bonuses to kingdom totals. The checkbox is off by default, and saved settlement totals are preserved; Defense/Unrest remain preview-only and costs are not yet applied. Schema v8 persists map grid dimensions, explicit zero-based rectangular building placements, visual labels as explicit non-overlapping regions, and manual checklist completion by turn; schema v9 adds calendar notes by source month occurrence. Schemas v1-v7 load with no prior checklist history; schema v8 preserves checklist history and migrates with empty calendar notes. Validation rejects invalid footprints, out-of-bounds items, and overlaps. Lot counts do not determine placement footprints. **Edit Map** provides a visual grid for 1×1 through 20×20, with inventory-backed or typed building names and terrain/corridor labels. Grid cells show label initials; full label text appears in the map-item list. Labels are descriptive only and have no terrain mechanics. Custom building names label placements only; they do not change inventory or catalog totals. **Load Template** offers optional `Capital Map`, `City # Map`, `Example Map`, and `Multi-District Map` layouts from the workbook. Replacing a populated draft requires confirmation, and the settlement changes only if the user selects **Apply**. The example's lot-count captions remain visual annotations, not general footprint rules. The source-defined terrain reference table is extractable with `tools/import_terrain_reference.py` into `assets/terrain_reference.json`; its values remain raw reference data and are not applied as map mechanics. Qt is not installed, so the desktop shell uses Win32. This remains a partial migration rather than full workbook parity; XLSX import is available through the CLI importer and reports unsupported formulas explicitly. JSON parsing uses the vendored nlohmann/json 3.11.3 single header, so the C++ runtime has no external dependency.

## Build with Visual Studio Code
1. Install Visual Studio 2022/2026 C++ Desktop Development workload, Windows SDK, CMake, and the VS Code CMake Tools extension.
2. Open this folder in VS Code, run **CMake: Select a Kit**, choose a Visual Studio x64 kit, then **CMake: Configure** and **CMake: Build** (targets `pkkm_desktop` and `pkkm_tests`).
3. Run **CMake: Run Tests**. Launch `build/Release/pkkm_desktop.exe` for a Visual Studio multi-config kit, or the executable in the selected build configuration directory.

CLI alternative from a Visual Studio Developer Command Prompt:
```
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tests/test_building_catalog_import.py
python tests/test_catalog_cli.py build/Release/pkkm.exe assets/building_catalog.json
```

Import a workbook copy into schema-v9 JSON and emit an auditable mapping report:

```text
python tools/import_workbook.py "C:/path/to/Kingdom of Template.xlsx" build/imported.json --report build/import-report.json --catalog assets/building_catalog.json
```

The importer maps direct kingdom, law, leadership, settlement, inventory, workbook-map, and Calendar note inputs. It does not evaluate spreadsheet formulas; every formula cell is listed in the report, along with manual workflow and reference-sheet limitations. The source workbook is never written, and the CLI rejects any output path that resolves to the source workbook.

The native JSON save path also rejects `.xlsx`, `.xlsm`, and `.xls` destinations, preventing the desktop application from overwriting a spreadsheet source by mistake.

Create the portable Release archive with the repository script:

```text
python tools/package_release.py --build-dir build/Release --output build/deliverables/PKKM-current-schema9.zip
```

The script validates and archives exactly `pkkm_desktop.exe`, `pkkm.exe`, and `building_catalog.json`; temporary smoke fixtures and test binaries are excluded.

The CMake project also exposes an install-level CPack ZIP package. After configuring and building, run `cpack --config build/asset-copy-verify/CPackConfig.cmake -G ZIP -B build/deliverables`. The generated `PKKM-0.1.0-Windows.zip` contains the same three end-user files; `tests/test_cpack_package.py` verifies the payload and rejects duplicates or test binaries.

The `pkkm` console target accepts `new <file.json>`, `show <file.json>`, and `catalog <catalog.json> [query]`. `show` prints Control DC, population, settlement totals when present, catalog-derived lot/district/population values, and either applied catalog stat bonuses or a clearly labeled preview, followed by stat totals and check thresholds. `catalog` searches workbook-backed improvements by name, or lists all when no query is supplied. Example: `build/Release/pkkm.exe catalog assets/building_catalog.json academy`. The native UI creates, opens, and saves JSON files and displays the calculated summary. **Kingdom Manager > Calendar Notes** preserves the source template's 85 month occurrences in source order and exact literal year markers. It records freeform notes under Holiday, Kingdom Upgrades, Events, and Other. Apply commits the page changes in memory and leaves the tracker open; Cancel reverts edits since the last Apply; Close returns to the main window. **Kingdom Manager > Turn Checklist** tracks the 20 source procedure items, including optional items, independently by turn; it records completion only and does not calculate checks or outcomes. Apply commits the page changes in memory, Cancel reverts edits since the last Apply, and Close returns to the main window. The unresolved Stability failure-by-exactly-4 case remains manual. **Kingdom Manager > Manage Settlements** edits settlement records and inventory; Apply commits the page changes in memory, Cancel reverts edits since the last Apply, and Close returns to the main window. **Kingdom Manager > Edit Inputs** follows the same Apply/Cancel/Close workflow. **Edit Map** accepts inventory selections or typed building names, explicit footprints, visual labels, and grids through 20×20. A schema-v7 GUI smoke used a temporary 15×15 map with 14 labels and a Castle 2×2 placement: the map was applied, saved, reopened through **File > Open**, and inspected again in **Edit Map**; CLI read-back confirmed the labels and placement persisted. A separate template smoke loaded **Multi-District Map** over a populated draft, showed 14 labels and no building placement, then cancelled without saving. Grid cells show label initials, with full text in the map-item list. Save with a `.json` extension. Data is written to a temporary file and renamed into place. A sample starter file is at `examples/starter.json`.

## Workbook source / coverage
The supplied template at `C:\Users\darks\Downloads\Kingdom of Template.xlsx` was read without modification. It contains Kingdom, Capital, Capital Map, Turn, City #, City # Map, Example Map, Multi-District Map, Calendar, and Data. The `IMP` named range (`Data!$A$2:$T$75`) supplies 70 improvement entries to `assets/building_catalog.json`; `tools/import_building_catalog.py` regenerates that derived asset without editing the workbook. The importer preserves source rows/cells and explicit row kinds; catalog search exposes only improvements. Regenerating the asset requires Python with `openpyxl`; the C++ runtime does not. Settlement records feed kingdom totals; catalog lots drive derived lots/districts/population, and opt-in catalog Economy/Loyalty/Stability bonuses affect kingdom totals while saved settlement totals remain intact. Schema v9 persists map dimensions, non-overlapping placements, visual labels, manual Turn checklist completion by turn, and Calendar Notes keyed to source month occurrences. The Win32 editor supports grids through 20×20 and optional presets for all four workbook map sheets. The Calendar tracker preserves 85 source entries and three literal year markers without date arithmetic. Those map sheets define no terrain mechanics. The manual checklist does not automate outcomes. Rule provenance is documented in `WORKBOOK_RULES.md`.
