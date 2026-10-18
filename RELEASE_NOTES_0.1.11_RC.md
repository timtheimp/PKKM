# PKKM v0.1.11 Release Candidate

## Summary

PKKM v0.1.11 packages the manual PDF city-layout worksheet refinements and the explicit rules boundaries established after v0.1.10.

## Included

- PDF City Layout `6x6 / 3x3 districts` template with visual D1-D9 district markers.
- Pure reference grouping for the nine visual 2x2 districts.
- Explicit, manual building placement with no inferred footprints, adjacency enforcement, legality calculation, or automatic city-stat effects.
- Manual commissioned magic-item workflow and manual settlement item handling.
- Caller-supplied terrain route costs and derived effects.
- Manual event consequences requiring explicit GM confirmation for non-numeric effects.
- Existing schema-v10 persistence, native Win32 workflows, PDF Worksheet fields, Greenbelt behavior, and valuable-item sales.

## Verification target

- Release desktop, CLI, and test targets built successfully.
- CTest domain and Python/import/catalog gates passed.
- Fresh native acceptance confirms the PDF City Layout template renders a 6x6 grid with D1-D9 in the expected 3x3 arrangement.
- Portable archive payload must contain exactly `pkkm_desktop.exe`, `pkkm.exe`, `building_catalog.json`, and `Kingmaker-Greenbelt-Map.jpg`.
- Publication, tagging, and asset upload remain separate approval actions. Existing releases `v0.1.8`, `v0.1.9`, and `v0.1.10` remain unchanged.
