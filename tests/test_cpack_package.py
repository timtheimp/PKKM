"""Verify the CPack install package contains only the end-user payload."""

import argparse
from pathlib import Path
import zipfile


EXPECTED = {"pkkm.exe", "pkkm_desktop.exe", "building_catalog.json"}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    args = parser.parse_args()
    with zipfile.ZipFile(args.archive) as archive:
        names = archive.namelist()
    if set(names) != EXPECTED or len(names) != len(EXPECTED):
        raise SystemExit(f"Unexpected CPack payload: {names}")
    print(f"CPACK_PACKAGE_PASS {args.archive}")
    print(f"CPACK_ENTRIES {names}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())