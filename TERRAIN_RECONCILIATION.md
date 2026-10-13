# Terrain and Corridor Reconciliation

Status: source-reconciliation artifact only. This document does not authorize automatic BP spending, terrain mutation, route mutation, or kingdom-stat changes.

## Source profiles

PKKM retains two parallel sources:

- **Workbook:** `Turn!A38:D47`, preserved verbatim in `assets/terrain_reference.json` as source rows 38–47.
- **Supplemental terrain reference:** the external Pathfinder terrain-improvements reference listed in `WORKBOOK_RULES.md`.

The supplemental page reviewed for this reconciliation is: <https://www.d20pfsrd.com/gamemastering/other-rules/kingdom-building/settlements/terrain-improvements/>.

The workbook table remains the source-faithful baseline for exploration time, preparation time/cost, and farm/road reference costs. The supplemental reference is a candidate source for prerequisites, improvement effects, and corridor rules only after reconciliation.

## Directly aligned reference values

The following workbook values match the supplemental terrain table for the comparable rows:

| Terrain | Exploration | Preparation | Farm / Road |
|---|---:|---:|---:|
| Cavern | 3 days | 3 months / 8 BP | — / 4 BP |
| Desert | 2 days | 1 month / 4 BP | 8 BP / 4 BP |
| Forest | 2 days | 2 months / 4 BP | — / 2 BP |
| Hills | 1 day | 1 month / 2 BP | 4 BP / 3 BP |
| Jungle | 2 days | 4 months / 12 BP | — / 4 BP |
| Marsh | 3 days | 3 months / 8 BP | — / 4 BP |
| Mountains | 3 days | 4 months / 12 BP | — / 4 BP |
| Plains | 1 day | Immediate / 1 BP | 2 BP / 1 BP |
| Water | 2 days | — | — |

These values are reference data. They do not imply that PKKM should calculate preparation progress, spend BP, or mutate a hex.

## Reconciled supplemental candidates

The supplemental reference provides additional rules that are not represented as executable workbook mechanics:

- A Farm is allowed in desert only with a canal, coastline, or river; Hills and Plains are also listed as valid Farm terrain.
- A Farm requires proximity to a river, lake, swamp, or Canal, or adjacency to at least two existing Farms.
- A Road may be built on any land hex; river crossings double the listed Road cost.
- A Highway requires kingdom Size 26 or greater and upgrades a Road; Highway and Road effects depend on connected route length.
- Aqueducts require a connected water source and settlement endpoint.
- Canals, Fishery, Fort, Mine, Quarry, Sawmill, Watchtower, and special-terrain effects introduce additional prerequisites or kingdom-stat effects.
- Some improvements may share a hex, while others have explicit exclusivity or upgrade relationships.

## Known catalog conflicts

The current Greenbelt catalog is intentionally conservative and does not yet
claim full supplemental coverage. In particular:

- The current Farm prerequisite text is narrower than the supplemental Farm
  terrain list and does not encode the desert water/Canal condition.
- The current Canal terrain requirement is water/coast, while the supplemental
  terrain list places Canals on desert, hills, and plains/grassland hexes.
- The current Fishery prerequisite text does not enumerate river and marsh
  terrain.
- The current Mine and Quarry terrain requirements are narrower than the
  supplemental terrain lists; the current Sawmill requirement is broader than
  the supplemental Forest-or-Jungle restriction.
- The current Road and Highway records add a claimed-land UI boundary and
  describe manual connectivity, but do not model river crossings, upgrade cost
  differences, or route length.

These are recorded conflicts, not silent corrections. The workbook table and
existing catalog remain unchanged until a source profile and executable rule
boundary are approved.

These rules require structured terrain topology, adjacency, route connectivity, improvement lifecycle, and source-profile decisions before they can become automatic behavior.

## Current PKKM boundary

The current Greenbelt catalog and editor may display these improvements as manual/reference records and previews. They must not:

- deduct BP or alter Treasury;
- mutate Consumption, Economy, Loyalty, Stability, Defense, or Unrest automatically;
- infer rivers, lakes, roads, connected routes, or settlement endpoints from labels;
- infer upgrades from an existing improvement name;
- convert a visual corridor or terrain label into a rule-bearing object;
- treat workbook cost strings as executable payment instructions.

The domain now exposes an additive, non-mutating preview overload that accepts
explicit caller-supplied facts for terrain, claim state, water/river/Canal
adjacency, neighboring Farms, Road connectivity, river crossings, and kingdom
Size, plus existing improvement IDs, a requested upgrade, and route length. It
can preserve the reconciled Farm, Fishery, and Highway warnings, including the
requirement for an existing Road when a Highway upgrade is requested, while
leaving all effects and costs manual. It also warns when a non-shareable
improvement already occupies the hex, permits existing shareable records, and
checks the Farm alternative of water access or adjacency to two existing Farms.
The preview also accepts the reconciled supplemental terrain sets for Canal,
Mine, Quarry, and Sawmill without changing the conservative catalog definitions.
It preserves the source occupancy warning that a Watchtower cannot share a hex
with a Fort or another Watchtower. Aqueduct previews additionally accept
explicit water-source and settlement-endpoint facts and warn when either is
absent. Bridge previews require an explicit river-crossing fact, while Road
previews report supplied river crossings without applying their cost effects.
Fort upgrade previews require an explicit existing Watchtower fact; ordinary
Fort placement remains distinct from that upgrade path.

## Safest next implementation slice

The first non-mutating terrain-improvement preview slice is implemented with explicit caller-supplied facts:

1. terrain type;
2. claimed status;
3. river/coast/water, settlement-endpoint, and river-crossing facts;
4. neighboring Farm count;
5. connected Road/Highway facts;
6. kingdom Size;
7. existing improvements, requested upgrade, and route length;
8. rule-profile selection remains a deferred boundary until profile-specific
   terrain semantics are approved.

The preview returns eligibility warnings and source metadata only. It does not spend BP, alter inventory, mutate map geometry, or apply kingdom-stat deltas. Automatic application remains deferred until workbook authority, City Info conflicts, and topology semantics are explicitly resolved.

The facts-based preview now accepts the persisted `RuleProfile` value and
records its lowercase name in the preview summary. This is source context
only; it does not silently apply profile-specific terrain semantics. The
returned preview also exposes the selected profile as structured context so
callers do not need to parse the summary text.
