# PKKM v0.1.7 Release Candidate

Status: release candidate preparation only. This file does not publish a GitHub release.

## Included since v0.1.6

- Native Greenbelt map selection with calibrated pointy-top hex geometry.
- Explicit `(0, 0)` anchor and visible calibration controls for temporary alignment.
- Large-map pop-out with zoom, Fit, drag panning, and persisted calibration support.
- Single-click selection without record creation.
- Deliberate double-click creation and removal, with confirmation for claimed or settlement-linked hexes.
- Advisory terrain detection with editable final terrain values and detector notes.
- Terrain token support for values such as `Plains, River`.
- Colored improvement labels, compact symbols, clipping, and settlement-name annotations.
- Stable settlement IDs and persisted settlement-to-hex links.
- Continuous blue outer perimeters for connected claimed territory and separate perimeters for disconnected groups.
- Muted internal claimed-cell borders and distinct settlement-linked visual accents.
- Regression coverage for shared borders, complete-cell bounds, hit-testing, and perimeter side mapping.

## Deliberate exclusions

- No automatic BP spending or refunds.
- No automatic route, topology, or map mutation.
- No automatic inventory or kingdom-stat mutation.
- Terrain recognition remains advisory and does not establish authoritative terrain or topology.
- Detailed improvement artwork remains deferred; current symbols are render-only.
- No release tag or GitHub publication is created by this candidate-preparation slice.

## Verification gate

The candidate must pass the local Release build, CTest, Python suite, `git diff --check`, deterministic portable package validation, and CPack payload validation before any tag or GitHub release is considered.
