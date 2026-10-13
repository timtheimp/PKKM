# Terrain Profile Decision Draft

Status: **Draft for approval only**. This document does not authorize automatic BP spending, route construction, map mutation, inventory mutation, or kingdom-stat changes.

## Purpose

PKKM now has source-reconciled, non-mutating terrain previews. The remaining boundary is deciding which terrain rules are authoritative under the persisted Workbook and City Info profiles. This draft records the decision needed before any terrain rule becomes state-mutating behavior.

## Proposed profile boundary

| Area | Workbook profile | City Info profile | Current implementation boundary |
|---|---|---|---|
| Exploration and preparation rows | Preserve `Turn!A38:D47` values | Preserve as a parallel reference | Reference-only |
| Farm, Road, and Highway costs | Preserve workbook reference values | Retain as a separate source candidate | Caller-supplied/manual |
| Terrain eligibility | Use only rules explicitly present in the workbook | Use only rules explicitly present in City Info or an approved supplemental source | Preview warnings only |
| Improvement effects | Do not infer from catalog descriptions | Do not infer from catalog descriptions | Manual |
| Route topology and connected length | No automatic inference | No automatic inference | Explicit facts and summaries only |
| Settlement endpoints and water topology | No automatic inference | No automatic inference | Explicit facts and warnings only |
| BP payment and refunds | No automatic payment rule | No automatic payment rule | Manual |
| Kingdom-stat changes | No automatic deltas unless a source gives the complete branch | No automatic deltas unless a source gives the complete branch | Manual |

## Preview rules currently available

These are additive preview checks only. They do not establish profile authority or permit automatic application:

- Farm terrain and water/river/Canal or two-Farm adjacency.
- Fishery terrain candidates, including river and marsh terrain.
- Canal supplemental terrain candidates.
- Mine, Quarry, and Sawmill supplemental terrain candidates.
- Aqueduct water-source and settlement-endpoint facts.
- Bridge river-crossing facts.
- Road river-crossing reporting without cost application.
- Highway connected-Road, Size 26, and Road-upgrade prerequisites.
- Fort upgrade from an explicit Watchtower fact.
- Watchtower exclusion with Forts and other Watchtowers.
- Non-shareable improvement occupancy warnings.
- Shareable-improvement occupancy acceptance.

## Decisions required before mutation

1. **Profile authority:** For each prerequisite, identify Workbook, City Info, both, or manual-only authority.
2. **Payment timing:** State when BP is paid, whether failed preparation consumes BP, and whether partial preparation is allowed.
3. **Upgrade semantics:** State whether Fort and Highway upgrades replace the source improvement, how costs differ, and whether the old record remains in history.
4. **Topology:** Define how roads, Bridges, Highways, Canals, Aqueducts, rivers, and settlement endpoints are represented and validated.
5. **Effects:** Provide complete source-defined kingdom-stat and settlement effects, including failure branches and reversals.
6. **Profile persistence:** Confirm that the selected profile is captured on every future terrain audit record, as it is for existing auditable actions.

## Recommended implementation order after approval

1. Persist and expose the approved terrain rule-profile selection in preview/audit context.
2. Add one pure reference function per approved prerequisite or upgrade rule.
3. Add auditable manual application only where payment, timing, failure, and reversal semantics are complete.
4. Keep all unresolved rules as warnings or explicit manual outcomes.
5. Run the Release build, CTest, Python suite, and package validation before considering a release slice.

Until these decisions are supplied, the prudent boundary is the current non-mutating preview model.
