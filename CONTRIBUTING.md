# Contributing to PKKM

Thank you for contributing to PKKM.

## Development environment

- Windows 11 or a compatible Windows environment
- Visual Studio C++ Desktop Development workload
- Windows SDK
- CMake 3.20 or newer
- Python 3.12 or newer
- Python package: `openpyxl`

## Build and test

From the repository root:

```text
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target pkkm pkkm_desktop pkkm_tests
ctest --test-dir build -C Release --output-on-failure
python tests/test_building_catalog_import.py
python tests/test_catalog_cli.py build/Release/pkkm.exe assets/building_catalog.json
python tests/test_terrain_reference.py
python tests/test_workbook_import.py
```

The GitHub Actions workflow runs the same Release and packaging checks on Windows.

## Workbook safety

The source workbook is specification input and must remain read-only. Importers may create derived JSON and report files, but must not overwrite or publish the source workbook.

Unsupported formulas and manual workbook workflows must remain explicit in reports. Do not replace them with guessed calculations.

## Change guidelines

- Preserve schema migrations and backward-compatible JSON loading.
- Keep map footprints explicit; do not infer dimensions from catalog lot captions.
- Keep Calendar Notes manual and source-faithful.
- Keep catalog Economy/Loyalty/Stability effects opt-in and disabled by default.
- Add or update tests with behavior changes.
- Do not commit build output, credentials, tokens, or local configuration.

Use focused commits with clear messages. Pull requests should describe the behavior changed and the verification performed.
