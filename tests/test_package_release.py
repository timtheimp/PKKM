import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from package_release import create_archive


class PackageReleaseTests(unittest.TestCase):
    def test_create_archive_contains_only_release_payload(self):
        with tempfile.TemporaryDirectory() as directory:
            build_dir = Path(directory) / "Release"
            build_dir.mkdir()
            for name in ("pkkm_desktop.exe", "pkkm.exe", "building_catalog.json"):
                (build_dir / name).write_text(name, encoding="utf-8")
            (build_dir / "temporary-smoke.json").write_text("temporary", encoding="utf-8")
            output = Path(directory) / "PKKM-current-schema9.zip"

            create_archive(build_dir, output)

            with zipfile.ZipFile(output) as archive:
                self.assertEqual(
                    archive.namelist(),
                    ["pkkm_desktop.exe", "pkkm.exe", "building_catalog.json"],
                )
                self.assertEqual(archive.read("pkkm.exe"), b"pkkm.exe")

    def test_create_archive_rejects_missing_release_payload(self):
        with tempfile.TemporaryDirectory() as directory:
            build_dir = Path(directory) / "Release"
            build_dir.mkdir()
            output = Path(directory) / "release.zip"
            with self.assertRaises(FileNotFoundError):
                create_archive(build_dir, output)


if __name__ == "__main__":
    unittest.main()
