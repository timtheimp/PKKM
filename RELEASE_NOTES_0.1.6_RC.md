# PKKM v0.1.6 Release Candidate

Status: release candidate preparation only. This file does not publish a GitHub release.

## Included since v0.1.5

- Source-reconciled, non-mutating terrain-improvement previews.
- Explicit Farm water and two-Farm adjacency checks.
- Supplemental Canal, Mine, Quarry, and Sawmill terrain previews.
- Aqueduct water-source and settlement-endpoint facts.
- Bridge and Road river-crossing facts without automatic cost application.
- Highway and Fort upgrade prerequisite previews.
- Watchtower/Fort occupancy warnings and non-shareable improvement warnings.
- Persisted rule-profile context in terrain previews and the native Greenbelt UI.
- Workbook and City Info profile regression coverage.
- Terrain profile decision draft documenting deferred automatic mechanics.

## Deliberate exclusions

- No automatic BP spending or refunds.
- No automatic route, topology, or map mutation.
- No automatic inventory or kingdom-stat mutation.
- No profile-specific terrain semantics beyond explicit preview context.
- No terrain improvement audit records or release publication in this candidate-preparation slice.

## Verification gate

The candidate must pass the local Release build, CTest, Python suite, `git diff --check`, deterministic portable package validation, and CPack payload validation before any tag or GitHub release is considered.
