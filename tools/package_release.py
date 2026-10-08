"""Create a portable PKKM release archive from a Release build directory."""

import argparse
from pathlib import Path
import zipfile


RELEASE_PAYLOAD = (
    "pkkm_desktop.exe",
    "pkkm.exe",
    "building_catalog.json",
)


def create_archive(build_dir: Path, output: Path) -> Path:
    """Write an archive containing exactly the supported portable payload."""
    build_dir = Path(build_dir)
    output = Path(output)
    missing = [name for name in RELEASE_PAYLOAD if not (build_dir / name).is_file()]
    if missing:
        raise FileNotFoundError(
            f"Release payload is missing from {build_dir}: {', '.join(missing)}"
        )

    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name in RELEASE_PAYLOAD:
            data = (build_dir / name).read_bytes()
            entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            entry.compress_type = zipfile.ZIP_DEFLATED
            entry.external_attr = 0o644 << 16
            archive.writestr(entry, data)
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    archive = create_archive(args.build_dir, args.output)
    with zipfile.ZipFile(archive) as verify:
        names = verify.namelist()
    if names != list(RELEASE_PAYLOAD):
        raise RuntimeError(f"Archive entries do not match release payload: {names}")
    print(f"PKKM_ARCHIVE_PASS {archive}")
    print(f"ARCHIVE_ENTRIES {names}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
