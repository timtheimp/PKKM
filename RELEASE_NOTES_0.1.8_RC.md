# PKKM v0.1.8 Release Candidate

Status: release candidate preparation only. This file does not publish a GitHub release.

## Included since v0.1.7

- Greenbelt Map Editor consolidation with direct Kingdom Manager entry.
- Claimed and mapped hex list relocated into the Map Editor right panel.
- Single-click selection of existing hexes with synchronized detail fields.
- Safe handling of blank Claim turn values without exposing a raw `stoll` error.
- Terrain notes persisted per Greenbelt hex.
- Custom improvement creation with deterministic normalized IDs.
- Editable effect and cost entry lists with catalog-backed dropdown choices.
- Multiple effect and cost entries with explicit Add, Update, and Remove behavior.
- Additive schema-v10 persistence for effect and cost arrays.
- Legacy custom-improvement migration from preview and cost description fields.
- Regression coverage for multi-entry round trips, legacy migration, and empty-entry rejection.

## Deliberate exclusions

- No automatic BP spending or refunds.
- No automatic route, topology, terrain, or map mutation.
- No automatic inventory or kingdom-stat mutation.
- Custom improvement effects and costs remain manual/render-only metadata.
- Terrain recognition remains advisory and does not establish authoritative terrain or topology.
- Exact-four Stability handling remains manual.
- No release tag or GitHub publication is created by this candidate-preparation slice.

## Verification gate

The candidate must pass the local Release build, CTest, Python suite, `git diff --check`, deterministic portable package validation, and CPack payload validation before any tag or GitHub release is considered.
