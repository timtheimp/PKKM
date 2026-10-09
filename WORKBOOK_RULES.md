# Workbook rule mapping — first calculation slice

Source: `C:\Users\darks\Downloads\Kingdom of Template.xlsx`, inspected read-only. The formula cells below are copied as workbook provenance; behavior is translated into tested C++ rather than evaluated by an Excel runtime.

## Kingdom statistics (`Kingdom!B5:B7`)

Each ability total follows `SUM(Cn:Gn)-Hn-In+Jn`.

| Input | Workbook column | Calculator field |
|---|---|---|
| Events | C | `events` |
| Alignment | D | `alignment` |
| Improvements | E | `improvements` |
| Leadership | F | `leadership` |
| Laws | G | `laws` |
| Unrest deduction | H | `unrest` |
| Vacancy deduction | I | `vacancies` |
| Other modifier | J | `other` |

Translated total: `events + alignment + improvements + leadership + laws - unrest - vacancies + other`.

The label formulas in `Kingdom!A5:A7` derive the check threshold from the Control DC in `Kingdom!G2` and the ability total in the corresponding B cell:

`IF((ControlDC - total) > 1, (ControlDC - total - 1) * 5, 5)`

The C++ API `pkkm::calculate_stat(KingdomStatInput, control_dc)` returns both the total and the threshold percentage. The percentage is not capped, matching the source formula. Arithmetic is widened before calculation and out-of-range results are rejected.

## Alignment bonuses (`Kingdom!D5:D7`)

The workbook tests the first and last character of the validated alignment code. A leading `L` adds +2 Economy; a trailing `E` adds +2 Economy; leading `C` and trailing `G` each add +2 Loyalty; leading `N` and trailing `N` each add +2 Stability. The single-character `N` therefore awards +4 Stability. Accepted values are the workbook's validation list: `LG, NG, CG, LN, N, CN, LE, NE, CE`.

Implemented by `pkkm::alignment_bonuses`; invalid codes are rejected.

## Edict/law effects (`Kingdom!B12:B20`)

| Input | Workbook cell | Effect implemented |
|---|---|---|
| Promotion law | B10 -> B12 | Stability: None −1, Token +1, Standard +2, Aggressive +3, Expansionist +4 |
| Promotion consumption | B10/B11 -> B13 | None 0, Token 1, Standard 2, Aggressive 4, Expansionist 8; Cathedral halves with Excel `ROUNDUP` behavior |
| Taxation law | B14 -> B16:B17 | Economy: None 0, Light +1, Normal +2, Heavy +3, Overwhelming +4. Loyalty: None +1, then −1/−2/−4/−8; Waterfront halves the non-None loyalty value with rounding away from zero |
| Holiday interval | B18 -> B19:B20 | `>None` sentinel (represented as 0 months): Loyalty −1, consumption 0. Intervals 1/6/12/24 months: Loyalty +1/+2/+3/+4 and consumption 1/2/4/8 |

The typed C++ inputs model workbook selections; only the stated validation-list values are accepted. `pkkm::calculate_laws` returns separate Economy, Loyalty, Stability, and consumption effects. Consumption remains separate because it is not a kingdom-stat bonus.

## Leadership and vacancies (`Kingdom!F5:F7`, `I5:I7`)

The three base leadership bonuses follow the workbook role groupings:

- Economy: Magister + Marshal + Treasurer (`G17 + G18 + G21`).
- Loyalty: Councilor + Royal Enforcer + Warden (`G13 + G19 + G22`).
- Stability: General + Grand Diplomat + High Priest (`G14 + G15 + G16`).

Baron's bonus (`G11`) is added to every stat selected by `H11` (Economy, Loyalty, Stability, or the workbook's supported combined labels). Spymaster's bonus (`G20`) is added only to the single stat selected by `H20`.

Vacancy deductions count missing name/leader entries: Economy gets 4 each for Magister, Marshal, Treasurer; Loyalty gets 4 for General and 2 each for High Priest and Warden; Stability gets 2 each for Grand Diplomat and Warden. Other vacant roles do not contribute to these three vacancy formulas. The domain API exposes role occupancy separately from manual bonus values, matching the source's text-presence tests.

Implemented by `pkkm::calculate_leadership` with typed Baron coverage and Spymaster focus.

## Control DC and population (`Kingdom!G2`, `J38`, `I38`, `F39`)

`ControlDC = 20 + kingdom size (E2) + total city districts (J38) + other (H2)`.

`Population = total city population (I38) + (kingdom size (E2) - city count (F39)) * 250`.

Implemented by `pkkm::control_dc` and `pkkm::population_total`; these functions preserve the source arithmetic without adding unverified clamps.

## Settlement aggregation (`Kingdom!D29:J39`, `Capital` / `City #`)

The kingdom sheet lists settlement sheet names in `D29:D37`. Each populated row reads that sheet's Economy/Loyalty/Stability totals (`E37:G37`), Defense (`H37`), population (`C4`), and districts (`C3`) into columns `E:J`. Row 38 sums those values; `F39=COUNTA(D29:D37)` counts the named settlements. The kingdom stat formulas add the settlement sums to the corresponding improvement inputs (`Kingdom!E5=E38+C54+C72`, `E6=F38+D54+D72`, and `E7=G38+E54+E72`).

`pkkm::aggregate_settlements` sums the stored per-settlement stat/defense fields. Its catalog-aware overload also sums inventory lots and derives effective districts and population. When at least one settlement record exists, `calculate_kingdom_summary` uses its count, effective population, effective districts, and stat totals instead of the manually stored territory aggregates; this avoids adding the same cities twice. With no records, the prior territory inputs remain the fallback. Defense is aggregated and displayed, but is not yet used by a kingdom-level rule.

The workbook derives `D37` as the sum of building lots (`Data!IMP` lots multiplied by each inventory count). Effective districts (`C3`) use the override only when `D3 > D37/36`; otherwise they use `ROUNDUP(IF(D37=0,1,D37/36),0)`. Effective population (`C4`) is the greater of the population override and `D37*250`. `pkkm::calculate_settlement_size` and catalog-aware settlement aggregation implement those formulas with checked integer bounds. The CLI and desktop dashboard use the bundled catalog; the manager labels districts/population as overrides. On the source sheets, rows 11–36 look up each selected building's effects from `IMP`, and row 37 sums them: `E37:G37` are Economy/Loyalty/Stability, `H37` Defense, and `I37` Unrest. Workbook totals are inventory-derived. To avoid double-counting migrated legacy totals, schema v5 adds a per-settlement opt-in (off by default) that applies only catalog Economy/Loyalty/Stability to kingdom totals without rewriting saved settlement fields. Defense/Unrest remain preview-only; building costs remain unapplied. The latest GUI smoke confirmed the opt-in checkbox and dashboard result, but did not save/reopen that changed setting. Prior manager Apply → Save → File > Open verification covered settlement name, population, districts, and inventory via UIA ValuePattern; ordinary key injection did not stick, so keyboard-entry behavior remains unverified.

## Improvement catalog (`Data!IMP`)

The workbook's `IMP` defined name resolves to `Data!$A$2:$T$75`. The importer reads the 20 headers from row 1 and preserves each nonempty source row number and its raw A:T cell values in `assets/building_catalog.json`. The supplied workbook copy exports 70 improvement records. Columns map as follows: A name, B cost, C lots, D Economy, E Loyalty, F Stability, G Defense, H Unrest, I Base Value, J Discount, K Magic Item, L Upgrade From, M Upgrade To, and N:T Corruption/Crime/Law/Lore/Society/Productivity/Fame. Columns outside the named range are not treated as building data.

The importer assigns explicit `improvement`, `basic`, `town`, `city`, `district`, or `unclassified` row kinds. Exact `Basic`, `Town`, `City`, and `District` labels are retained as non-building rows and never surfaced by `search_building_catalog`; they are not silently flattened into building definitions. `pkkm::load_building_catalog` retains source row, kind, and raw cells, while `pkkm::search_building_catalog` returns only improvement entries. The CLI exposes this reference slice with `pkkm catalog <catalog.json> [query]`. Regenerate only the derived asset with:

```text
python tools/import_building_catalog.py "C:/Users/darks/Downloads/Kingdom of Template.xlsx" assets/building_catalog.json
```

The Win32 settlement manager uses catalog improvements to add/remove per-settlement building counts and displays each selected improvement's cost and lot size. Schema v5 persists inventory names/counts and the opt-in flag; older saves default the flag off. Catalog lots drive effective settlement lots, districts, and population through the formulas above. When enabled, catalog Economy/Loyalty/Stability effects contribute to kingdom totals while saved settlement stat fields remain unchanged; Defense and Unrest are preview-only. This explicit policy preserves legacy totals and prevents implicit double-counting. Costs remain unapplied.

## Settlement map sheets (`Capital Map`, `City # Map`, `Example Map`, `Multi-District Map`)

Read-only inspection of `Capital Map`, `City # Map`, `Example Map`, and `Multi-District Map` found zero formulas and zero cell data validations. These sheets specify visual merged-cell layouts, not terrain bonuses, movement costs, or corridor mechanics. Convert a merged range to the map model as zero-based `(row=min_row-1, column=min_column-1, width, height)`; the merged range, not a lot-count label, establishes that region's displayed footprint.

The 8×8 `Capital Map` and `City # Map` share these border annotations: `B1:G1` Hill; `A2:A7` Forest; `H2:H7` Water; `B8:G8` Water. `Capital Map` also shows Castle at `F6:G7`, a 2×2 region (`row=5, column=5, width=2, height=2`). `Example Map` repeats the border layout and contains `2x lots` in `B6:B7` (1×2), `To be built` in `C6` (1×1), and `4x lots eg. castle` in `F6:G7` (2×2). Those captions do not define general footprint rules; in this example, the actual merge range is what makes the region 2×2. The `City # Map` sheet has no building placement.

The 15×15 `Multi-District Map` contains four 6×6 interior areas separated and bordered by these 14 merged annotations. PKKM coordinates below are `(row, column; width×height)`:

| Excel range | Text | PKKM region |
|---|---|---|
| `B1:G1` | Hill | `(0,1; 6×1)` |
| `I1:J1` | Hill | `(0,8; 2×1)` |
| `K1:L1` | Gate | `(0,10; 2×1)` |
| `M1:N1` | Hill | `(0,12; 2×1)` |
| `A2:A7` | Forest | `(1,0; 1×6)` |
| `H2:H7` | Forest | `(1,7; 1×6)` |
| `O2:O7` | Water | `(1,14; 1×6)` |
| `B8:G8` | Forest | `(7,1; 6×1)` |
| `I8:N8` | Hill | `(7,8; 6×1)` |
| `A9:A14` | Forest | `(8,0; 1×6)` |
| `H9:H14` | Forest | `(8,7; 1×6)` |
| `O9:O14` | Water | `(8,14; 1×6)` |
| `B15:G15` | Water | `(14,1; 6×1)` |
| `I15:N15` | Water | `(14,8; 6×1)` |

The schema-v7 smoke fixture's 14 labels were compared to these source ranges and matched exactly. Its Castle at `(5,5)` 2×2 was an added placement test; the workbook's `Multi-District Map` has no Castle placement. The Win32 **Edit Map** surface supports grids up to 20×20 and represents merged annotations as explicit label rectangles; grid cells display initials and the map-item list displays full text. **Load Template** provides optional presets for all four sheets: Capital includes its Castle placement, City contains only the four border labels, Example preserves its three captions as visual labels, and Multi-District contains the 14 regions above. Loading a populated draft requires confirmation; changes remain in the editor draft until **Apply**. Labels are visual only. Placement and label footprints remain explicit, non-overlapping, and independent of catalog lot counts. Custom building names label placements only; they do not change inventory or totals. Domain persistence supports larger positive dimensions, and the GUI now accepts them through 20×20.

## Turn and calendar sheets (`Turn`, `Calendar`)

Read-only inspection found no formulas or cell data validations on either sheet. `Turn` is a 74×4 static procedure outline with merged phase headings `A1:D1`, `A17:D17`, `A53:D53`, and `A71:D71`: Phase 1 Upkeep, Phase 2 Edicts, Phase 3 Income, and Phase 4 Events. It also contains reference tables for kingdom-size limits (`A29:D35`) and terrain exploration/preparation/farm-road costs (`A37:D47`). One conditional-formatting range, `C22:C24`, has numeric cell-is rules; this is formatting, not a formula-driven turn mechanic. The steps require manual decisions and contain an unresolved Stability-check boundary: the listed failure outcomes are “Fail by < 4” and “Fail by >= 5”, leaving failure by exactly 4 unspecified. Do not automate turn outcomes from this sheet without resolving that gap.

The desktop **Turn Checklist** mirrors the 20 named procedure items across those four phases, including all four optional tasks, and stores completed item IDs separately for each turn. It is a manual completion tracker only: it does not calculate checks, update kingdom values, or decide event/edict outcomes. The domain exposes `classify_stability_failure()` for the source's explicit `< 4` and `>= 5` branches; failure by exactly 4 returns `UnresolvedExactlyFour` and is not assigned an outcome. Schema v8 adds this checklist history; schema v1-v7 files migrate with no prior checklist records.

The explicit Turn rows for Claim Hexes, Abandon Hex, and Abandon City are exposed by `resolve_turn_action()` as pure deltas: Claim Hexes is `-1 BP, +1 Size`; Abandon Hex is `+1 Unrest, -1 Size`; and Abandon City is `+4 Unrest, -1 Size`. These helpers do not advance turns or mutate a kingdom automatically.

The kingdom-size table `Turn!A29:D35` is exposed by `kingdom_size_limits()` with exact inclusive bands `1–10`, `11–25`, `26–50`, `51–100`, `101–200`, and `201+`. It preserves settlement, building, improvement, and hex-claim limits, including the literal `No limit` building case as `unlimited_new_buildings=true`. Size zero or negative is rejected; no turn state is mutated.

The Phase 3 income reference rows are exposed by `turn_economy_rates()` without applying conversions: withdrawal uses `2000 gp` per `1 BP` and `+1 Unrest`; deposit uses `4000 gp` per `1 BP`; item deposits use `8000 gp` base value per `1 BP`; and Buy for kingdom uses `2000 gp` per `1 BP`. Rounding, partial units, and tax outcomes remain manual until the workbook specifies their semantics.

The tax rows expose `resolve_tax_bp_delta(success, result_divided_by_three, other_bp_gained)`: success returns the supplied result contribution plus other BP gained, while failure returns only other BP gained. The caller must supply the already-resolved `Result / 3` value; no rounding rule is invented.

The source-defined terrain reference rows `Turn!A38:D47` are preserved by `tools/import_terrain_reference.py` in `assets/terrain_reference.json` as raw strings with source row numbers. The extracted table is reference-only: it does not establish map terrain bonuses, movement mechanics, or automatic farm/road outcomes.

`Calendar` reports a 99×21 sheet dimension, but contains 93 nonempty cells, no merges, no formulas, no validations, no conditional formatting, no hidden rows/columns, and no tables. Headers are in `B1:F1`: Month, Holiday, Kingdom Upgrades, Events, Other. The Month column has 85 populated entries (`B2:B86`) and 12 distinct month names; the four note columns below their headers are blank. Only three year-marker labels are present (`A14` = `1 yr!`, `A26` = `2 years!`, `A38` = `year 3`), and the populated month sequence ends at `B86`. Treat this as an incomplete/manual calendar template: do not assume a complete year count, invent missing year markers, or derive turn progression from it.

The desktop **Calendar Notes** tracker preserves all 85 month occurrences in source order, including exact month strings and the three literal marker labels. Notes are keyed by source row (`calendar.row.2` through `calendar.row.86`), so repeated month names remain distinct. The four note fields map to Holiday, Kingdom Upgrades, Events, and Other. Schema v9 persists notes; schema v1-v8 loads with an empty note list. The tracker does not infer dates, fill missing markers, or advance turns. GUI Apply/Save/read-back verification was completed with a persisted Holiday note on `calendar.row.2`.

The full workbook importer is `tools/import_workbook.py`. It opens a workbook copy read-only and maps direct inputs from `Kingdom`, settlement sheets, their workbook map sheets, and `Calendar` into schema-v9 JSON. It uses the existing `IMP`-derived catalog to distinguish map building placements from visual labels. Formula cells are not recalculated or guessed: every formula is retained in the generated mapping report as unsupported/derived content, and `Turn` is reported as a manual workflow rather than converted into automated outcomes. The importer therefore produces both a usable JSON draft and an explicit account of workbook content that still requires manual handling.

## Current exclusions

Kingdom rule inputs remain persisted under `rules`; schema v1-v8 documents migrate to schema v9 with empty calendar-note history, while v1-v7 also receive empty checklist history and existing catalog-stat/map/label defaults. The CLI and dashboard distinguish applied bonuses from preview; the manager exposes the opt-in checkbox. The schema-v9 map model supports visual labels, and the 1×1–20×20 editor supports placements and labels, but terrain/corridor mechanics remain excluded. Catalog costs, automated turn outcomes, terrain/improvement aggregation, formula cell text/type semantics, and the workbook's `TEXT.LEGACY`-based displayed check label are also outstanding. Workbook formula evaluation is intentionally outside `tools/import_workbook.py`; those cells are emitted in its mapping report instead of being silently dropped.

## Verification

Tests in `tests/kingdom_tests.cpp` cover the worked total/check threshold, signed per-stat contribution breakdowns, alignment codes `N` and `CE`, invalid alignment rejection, combined edict/law effects, negative half-rounding, Cathedral consumption rounding, source-specific vacancies, staffed leadership bonuses with Baron/Spymaster benefits, settlement aggregation and summary integration, Control DC, population, schema-v9 settlement/map/checklist/calendar persistence, schema-v1-v8 migrations, calendar source row/order/text fidelity, separate repeated-month notes, calendar ID validation, checklist step validation and per-turn separation, label bounds and overlap validation, explicit placement footprints, all four workbook map presets and their merged regions, catalog row mapping/search filtering, catalog-derived lots/stat effects, and lot/district/population override boundaries. JSON integer fields outside the native `int` range are rejected rather than narrowed; inventory and catalog-derived totals include overflow checks. `tests/test_building_catalog_import.py` covers source-kind preservation and column mapping; `tests/test_catalog_cli.py` checks catalog output and opt-in/off status; `tests/test_package_release.py` checks the exact portable archive payload; `tests/test_workbook_import.py` checks direct-input mapping, workbook-map classification, Calendar notes, and formula reporting. The schema-v9 Release `pkkm_tests` build, focused `kingdom_domain` CTest, Calendar GUI save/read-back smoke, workbook import against the source template, and schema-v9 archive verification pass. Earlier map smoke applied/saved/reopened the schema-v7 fixture with 14 labels and one Castle placement; the template smoke loaded Multi-District and cancelled without saving. The source workbook remains untouched.

```text
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```
