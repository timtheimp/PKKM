# PKKM v0.1.12 Release Candidate

## Summary

PKKM v0.1.12 packages the seeded settlement random-item catalog/runtime path, guided turn-phase adapters, source-faithful army and city worksheet work, and responsive native-window layout refinements.

## Included

- Schema-v10 runtime packaging for `random_magic_item_catalog.json`.
- Seeded, auditable settlement magic-item preview, application, availability preview, and explicit manual-resolution states for deferred source branches.
- Guided Turn Checklist workflow ordered as Upkeep, Edicts, Income, and Events.
- Guarded checklist adapters and handoffs for claim hexes, withdrawals, deposits, item deposits, taxes, event rolls/resolution, construction, city worksheets, army worksheets, and law inputs.
- Source-faithful army roster and creation worksheet with caller-supplied manual combat fields and explicit BP resource costs.
- Responsive Turn Checklist layout with separated lower bands and a reserved Apply/Cancel/Close area.
- Responsive Greenbelt Map Editor with right-docked tool controls during resize and maximize.
- Responsive Turn Actions layout that reflows grouped controls and retains the scrollbar when the complete action surface exceeds the client height.
- Existing PDF-first rules profile, manual outcome boundaries, schema-v10 persistence, native Win32 workflows, and deterministic package validation.

## Verification

- Release build completed with Visual Studio 18 2026.
- CTest passed `2/2`.
- Native regression executable passed.
- CLI version reported `PKKM 0.1.12`.
- Deterministic package passed exact five-entry validation.
- CPack archive passed exact five-entry validation.
- Catalog CLI tests passed `6` tests.
- Random magic-item table tests passed `4` tests.
- Fresh native Turn Actions captures verified normal and maximized reflow plus scrollbar retention. Direct corner-drag geometry verification remains dependent on the desktop automation driver and is not claimed as complete.

Publication, tagging, and asset upload remain separate approval actions. Published `v0.1.11` remains unchanged.
