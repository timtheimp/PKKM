# PKKM Token-Saver Workflow

This project is intentionally developed with a staged verification budget. The goal is to reduce redundant context and test runs without weakening correctness.

## Repository facts

- C++17/CMake/Win32 application with framework-independent domain logic.
- One primary C++ domain test executable, plus four Python tests for the catalog importer, CLI, release packaging, and workbook import.
- `build/` contains generated Visual Studio/CMake output and must not be treated as source.
- `third_party/nlohmann/json.hpp` is a vendored dependency and is not read unless JSON-library behavior is the issue.
- The source workbook is read-only provenance. Derived catalog output may be regenerated; the workbook itself must not be overwritten.
- Current persistence is schema v9. Calendar Notes and Turn Checklist remain manual trackers.

## Test-budget tiers

1. **Inspect:** search symbols and read only relevant line ranges.
2. **Focused:** build the affected target and run the matching test selection.
3. **Milestone:** after a coherent feature slice, build Release targets and run full Release CTest plus only the relevant Python checks.
4. **GUI:** run only when acceptance depends on Win32 behavior that CLI/domain tests cannot prove. Capture fresh state after every state-changing action.
5. **Package:** rebuild the current schema package only after the milestone and required GUI checks pass; list the archive programmatically.

Do not run Tier 3 after every edit. Do not claim Tier 4 from an input call marked unverifiable. Do not package stale binaries.

## Canonical focused commands

```text
cmake --build build/asset-copy-verify --config Release --target pkkm_tests
ctest --test-dir build/asset-copy-verify -C Release --output-on-failure -R kingdom_domain
```

## Canonical milestone commands

```text
cmake --build build/asset-copy-verify --config Release --target pkkm_desktop pkkm pkkm_tests
ctest --test-dir build/asset-copy-verify -C Release --output-on-failure
python tests/test_building_catalog_import.py
python tests/test_catalog_cli.py build/asset-copy-verify/Release/pkkm.exe assets/building_catalog.json
python tools/package_release.py --build-dir build/asset-copy-verify/Release --output build/deliverables/PKKM-current-schema9.zip
```

## Operating rules

- Batch independent searches, reads, and read-only commands.
- Search before reading; read definitions, callers, serializers, and nearest tests only.
- Patch the smallest existing-file region; create new documents with `write_file`.
- Preserve schema migrations and workbook caveats; never invent dates, turn outcomes, map mechanics, or footprints.
- Remove temporary fixtures and terminate smoke-test processes before completion.
- Report the exact verification tier reached and any unverified GUI or packaging step.
