# Terrain and Corridor Reconciliation

Status: source-reconciliation artifact only. This document does not authorize automatic BP spending, terrain mutation, route mutation, or kingdom-stat changes.

## Source profiles

PKKM retains two parallel sources:

- **Workbook:** `Turn!A38:D47`, preserved verbatim in `assets/terrain_reference.json` as source rows 38–47.
- **Supplemental terrain reference:** the external Pathfinder terrain-improvements reference listed in `WORKBOOK_RULES.md`.

The supplemental page reviewed for this reconciliation is: <https://www.d20pfsrd.com/gamemastering/other-rules/kingdom-building/settlements/terrain-improvements/>.

The workbook table remains the source-faithful baseline for exploration time, preparation time/cost, and farm/road reference costs. The supplemental reference is the approved default for Greenbelt prerequisites where the profiles differ; improvement effects, route costs, and kingdom-stat effects remain manual unless separately defined.

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

The supplemental reference is the default when the parallel profiles differ. The following observations remain caller-supplied rather than inferred:

- A Farm is buildable when its terrain has a qualifying descriptor; water access, neighboring Farms, and other supplemental facts remain caller-supplied observations when recorded.
- A Road may be built on any land hex; river crossings double the listed Road cost. Road connectivity is supplied by the caller and connects to adjacent hexes that contain Roads. Road spacing uses the clarified 12 miles center-to-center, approximately 7 miles per side.
- A Highway requires kingdom Size 26 or greater and upgrades a Road; Highway and Road effects depend on connected route length.
- Aqueducts require a connected water source and settlement endpoint.
- Canals, Fishery, Fort, Mine, Quarry, Sawmill, Watchtower, and special-terrain effects introduce additional prerequisites or kingdom-stat effects.
- Some improvements may share a hex, while others have explicit exclusivity or upgrade relationships.

## Known catalog conflicts

The current Greenbelt catalog is intentionally conservative and does not yet
claim full supplemental coverage. In particular:

- The catalog retains source prerequisite text for auditability, while the
  executable Greenbelt prerequisite accepts a qualifying terrain descriptor;
  unresolved water/Canal and neighboring-Farm facts remain caller-supplied.
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

These are recorded source conflicts, not silent catalog rewrites. The workbook
table and catalog remain preserved for provenance while the approved executable
boundary uses the supplemental default only for the validated prerequisite and
inventory behavior described below.

Terrain descriptors are now sufficient for the terrain prerequisite: when a hex has a qualifying descriptor, the improvement is buildable. Topology facts remain caller-supplied for Roads, Highways, Bridges, Aqueducts, and related corridor checks. Applying a catalog improvement updates the Greenbelt hex inventory automatically; derived effects, BP spending, route costs, and kingdom-stat changes remain separately manual unless defined elsewhere.

## Current PKKM boundary

The current Greenbelt catalog and editor may display these improvements as manual/reference records and previews. They must not:

- deduct BP or alter Treasury;
- mutate Consumption, Economy, Loyalty, Stability, Defense, or Unrest automatically;
- infer rivers, lakes, connected routes, or settlement endpoints from labels; Road adjacency remains a caller-supplied observation;
- infer upgrades solely from an existing improvement name; an upgrade must be
  explicitly requested and validated against its source improvement;
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
accepts the supplemental terrain descriptors for Farm, Fishery, Canal, Mine, Quarry, and Sawmill.
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
8. the supplied Kingmaker PDF is the overall default rules profile; the supplemental terrain reference is the approved default specifically for Greenbelt prerequisites where it differs from the workbook, and explicit `pdf`, `workbook`, or `city_info` profile context remains available to the domain API.

The preview returns eligibility warnings and source metadata only. Applying a qualifying improvement updates the selected hex inventory and replaces the explicit Road or Watchtower source for Highway or Fort upgrades. It does not spend BP, alter map geometry, or apply kingdom-stat deltas. Automatic route costs and derived effects remain deferred.

The facts-based preview uses the supplemental terrain reference as the default authority specifically for Greenbelt prerequisites where it differs from the workbook. It accepts explicit `pdf`, `workbook`, or `city_info` profile context for the surrounding kingdom rules, records the selected profile name in the preview summary, and exposes it as structured context so callers do not need to parse summary text.
