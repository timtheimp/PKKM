# PKKM v0.1.9 Release Candidate

## Summary

PKKM v0.1.9 packages the verified native construction and Greenbelt workflow slice merged after v0.1.8.

## Included

- Atomic multi-line construction transactions with BP totals, discounts, upgrades, placement validation, building limits, failed-check overrides, audit records, persistence, and explicit reversal.
- Native Win32 Construction Transactions workflow with staging, preview, finalization, history, and guarded reversal.
- Construction reversal safeguards for dependent transactions, malformed audit records, current-turn limit accounting, and original upgrade footprints.
- Greenbelt Map Editor window placement, DPI alignment, viewport clipping, zoom/pan behavior, scroll synchronization, and monitor work-area clamping.
- Custom Greenbelt improvement creation, editing, collection refresh, and **Save + Add to Hex** behavior.
- Settlement Apply/Cancel protection around construction actions, including rejection of unapplied edits across all settlements.

## Verification

- Release desktop, CLI, and test targets built successfully.
- CTest: 2/2 passed.
- Building catalog tests: 2 passed.
- Catalog CLI tests: 4 passed.
- Workbook import tests: 2 passed.
- Package tests: 2 passed.
- Independent final review: passed with no findings.

## Release boundary

This file is a release-candidate note. Publication, tagging, and asset upload remain separate release actions. Existing release `v0.1.8` remains unchanged.
