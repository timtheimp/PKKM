# PKKM — Standalone Kingdom Manager

## Source analysis
Source workbook: `C:\Users\darks\Downloads\Kingdom of Template.xlsx`.

The workbook has 10 sheets: Kingdom, Capital, Capital Map, Turn, City #, City # Map, Example Map, Multi-District Map, Calendar, and Data. It contains substantial formula-driven kingdom statistics, law/edict and leadership modifiers, settlement/building tracking, city map layouts, a turn tracker, a 12-month calendar, and a building/improvement reference table. The workbook also uses merged cells, a named range (`IMP`), cross-sheet `INDIRECT` references, and Google Sheets compatibility formulas (`__xludf.DUMMYFUNCTION` / `TO_TEXT`). Those Excel formula constructs should not be copied mechanically; implement and test their intended behavior in native C++ domain logic.

## Objective
Build PKKM as a standalone Windows desktop application in C++, developed and built through Visual Studio Code. It should preserve the workbook's kingdom-management utility while replacing fragile spreadsheet formulas with explicit, testable application logic.

## Current implementation status
- **Foundation delivered:** Win32 desktop shell, C++17 core library, versioned JSON persistence, CLI, and always-on tests; see `README.md`.
- **Current calculation/data slices:** stat aggregation/check thresholds; alignment bonuses; promotion, taxation, holiday-law/edict effects; leadership bonuses/vacancy deductions; Control DC; population; settlement aggregation; catalog-derived settlement lots/population/districts; opt-in catalog Economy/Loyalty/Stability bonuses; settlement maps; four optional workbook map-layout presets; a manual Turn checklist; and a source-faithful Calendar Notes tracker. JSON schema v9 persists settlement/map state, checklist step IDs by turn, and calendar notes by unique source-row occurrence. Schemas v1-v7 migrate with empty checklist and calendar history; schema v8 preserves checklist history and migrates with empty calendar notes. Validation enforces positive footprints, grid bounds, non-overlap, unique checklist turns, known checklist IDs, and unique known calendar entry IDs. Neither checklist nor calendar performs turn/date arithmetic or resolves outcomes. Provenance and exclusions are in `WORKBOOK_RULES.md`.
- **Catalog, workbook import, and settlement editor:** `tools/import_building_catalog.py` reads the workbook's `IMP` range into `assets/building_catalog.json`, retaining source rows/cells and typed non-building records; `tools/import_workbook.py` imports supported kingdom, law, leadership, settlement, map, and Calendar note inputs into schema-v9 JSON and emits a mapping report listing every formula that was not evaluated. The C++ API and CLI search only improvement records. The Win32 manager edits settlement fields, inventory counts, and a per-settlement catalog-stat opt-in that defaults off. Catalog lots derive lots/district/population using workbook override rules. When opted in, only Economy/Loyalty/Stability bonuses affect kingdom totals; saved settlement totals are preserved, while Defense/Unrest remain preview-only and costs remain unapplied.
- **Verified:** Release `pkkm_desktop`, `pkkm`, and `pkkm_tests` built; the focused Release `kingdom_domain` CTest passed for schema v9. Domain coverage checks all 85 Calendar rows against the read-only source sequence, the three literal year markers, repeated-month note separation, validation, persistence, and schema-v8 migration. Calendar GUI Apply/Save/read-back was verified with a persisted Holiday note, `build/deliverables/PKKM-current-schema9.zip` was rebuilt with an exact three-file payload, and the CPack install ZIP was verified with the same payload. The source workbook remains untouched.
- **Next:** continue translating only explicit Turn rows into named domain actions; keep manual decisions and unresolved branches outside automation. Preserve the partial 85-entry source sequence and do not infer missing years or date progression. Terrain/corridor labels remain descriptive, and map captions such as `2x lots` and `4x lots` do not establish general footprints.
- **Not yet delivered:** full automated turn workflow, Stability outcomes beyond explicit branch classification, and terrain/corridor mechanics. The source-defined Claim Hexes, Abandon Hex, and Abandon City deltas, kingdom-size reference limits, income conversion rates, tax success/failure branch, magic-item optional branches, event cadence, and manual event-resolution boundary are now exposed as pure domain values; the exact-four Stability boundary remains `UnresolvedExactlyFour` rather than guessed. Tax rounding is caller-supplied because the workbook does not define it. Formula evaluation remains intentionally outside the importer; unsupported manual workflow content is reported rather than silently discarded. Catalog costs and effects beyond the approved Economy/Loyalty/Stability opt-in also remain unapplied. Workbook provenance is documented in `WORKBOOK_RULES.md`.

## Proposed architecture
- C++17 or newer; CMake project opened in Visual Studio Code.
- Native Windows desktop UI. Qt was unavailable in the supplied environment, so the current shell uses the Windows SDK/Win32 API; keep calculation/domain code framework-independent.
- Local-first persistence using versioned JSON (or SQLite if relational editing/history warrants it); no network dependency.
- Modules: domain models, rules/calculations, persistence/import-export, UI, automated tests.

## Functional scope (initial release)
1. **Kingdom dashboard:** identity/alignment/size, Economy/Loyalty/Stability, control DC, population, unrest, consumption, treasury, vacancies, leadership, laws/edicts, modifiers and improvements.
2. **Settlements:** multiple cities/towns, each with editable population/district overrides, stats, defense, and catalog-backed building inventory. Lot totals, minimum districts, and minimum population are catalog-derived; Economy/Loyalty/Stability catalog bonuses are opt-in per settlement and off by default.
3. **Building catalog:** the reference-data importer, CLI search, settlement inventory selector, lot-derived settlement sizing, and opt-in Economy/Loyalty/Stability aggregation are delivered. Catalog costs and other effects remain future work.
4. **Maps:** editable city district/lot grids; visual terrain/corridor labels and building placement; optional presets for the `Capital Map`, `City # Map`, `Example Map`, and `Multi-District Map` sheets. Preserve explicit merged-cell footprints. The map sheets do not define terrain mechanics. The example map's “2x lots” and “4x lots” captions must not be generalized into footprints; use only their specific merged ranges.
5. **Turns and calendar:** monthly turn log, events, holidays, kingdom upgrades and notes; advance turn without losing history.
6. **Persistence:** create/open/save kingdom files; autosave or clear unsaved-change indication; backup-safe writes.
7. **Spreadsheet migration:** read-only import from the supplied workbook or a populated copy, preserving values and mapping supported sheets. Report unsupported formulas/fields rather than silently dropping them. Export CSV/JSON initially; XLSX export is optional and should not block core application delivery.

## Calculation and data rules
- Translate formulas into named functions and document each rule with its workbook cell/formula provenance.
- Separate user-entered values, derived values, and overrides. Never overwrite manual input with a computed value.
- Handle blank/text/numeric states explicitly; this workbook uses text sentinels and formulas that rely on type checks.
- Confirm edge cases against workbook behavior: kingdom size/population thresholds, vacancy penalties, edict/law modifiers, building counts/lots, upgrades, and settlement aggregation.
- Do not claim formula parity for spreadsheet compatibility functions until their output is demonstrated by test cases.

## Delivery stages
1. Inventory all used cells, formulas, validations, formatting cues, named ranges, and sample map layouts; establish a requirements/cell-mapping matrix.
2. Define domain schema and calculation specifications; create representative golden test cases from workbook inputs and expected outputs.
3. Implement CMake scaffold, domain models, formula/rule tests, and versioned save/load.
4. Implement dashboard, kingdom laws/leadership, city management, building catalog, and turn/calendar workflows.
5. Implement district maps and migration/import/export; validate import against the workbook.
6. Build/debug in VS Code on Windows, run unit and integration tests, then package a standalone executable with a concise user guide.

## Acceptance criteria
- Builds reproducibly from a clean checkout using documented VS Code/CMake steps.
- Core kingdom and settlement calculations have automated tests for normal and boundary cases.
- A sample kingdom can be created, saved, closed, reopened, edited, and saved without data loss.
- Building catalog entries and map/turn records persist correctly.
- Import of a workbook copy yields a mapping report and does not silently discard unsupported content.
- No spreadsheet-engine runtime or internet connection is required for normal operation.
- Final review checks correctness, data safety, validation, error handling, and test/build results before release.

## Open decisions for implementation
- Confirm Qt 6 availability and whether a single-file Windows installer/portable folder is preferred.
- Confirm migration priority: blank template support first, or import of an already populated workbook as a release requirement.
- Confirm exact Kingmaker ruleset assumptions where the workbook is ambiguous; preserve documented workbook behavior unless Ty directs otherwise.
