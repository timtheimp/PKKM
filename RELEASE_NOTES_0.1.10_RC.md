# PKKM v0.1.10 Release Candidate

## Summary

PKKM v0.1.10 packages the PDF-default valuable-item sales workflow, hardened Greenbelt rule handling, and the associated audit, persistence, native, CLI, and XLSX reporting changes merged in PR #63.

## Included

- PDF rule profile as the default for new kingdoms where the supplied Kingmaker PDF governs the behavior.
- Persisted valuable-item sales with PDF Economy DCs, city-market validation, district cadence, BP results, failure records, native Turn Actions controls, CLI reporting, and XLSX export.
- Backward-compatible Workbook profile fallback for legacy kingdoms and audit records without an explicit profile.
- Checked arithmetic and atomic validation for tax, turn actions, withdrawals, Consumption, persisted records, and export values.
- Greenbelt terrain-token handling, active-profile propagation, explicit Road-to-Highway and Watchtower-to-Fort upgrades, upgrade metadata validation, and informational river-crossing route notes.
- Preserved Kingmaker PDF reference pages and images for the implemented rules boundary.

## Verification

- Release desktop, CLI, and test targets built successfully before merge.
- CTest: 2/2 passed.
- Python suite: 15/15 passed under `uv run --with openpyxl --python 3.11`.
- `git diff --check`: passed.
- Fresh native valuable-item acceptance: passed, including save/reload and persisted history display.
- GitHub Actions `release-windows` for PR #63: passed.

## Release boundary

This file is a release-candidate note. Tagging, publication, and asset upload remain separate release actions. Existing releases `v0.1.8` and `v0.1.9` remain unchanged.
