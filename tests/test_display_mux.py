import subprocess
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class DisplayMuxTest(unittest.TestCase):
    def test_display_scan_blanks_between_digits_and_uses_nonzero_tick_delay(self):
        binary = ROOT / "build" / "test_display_mux_harness"
        binary.parent.mkdir(exist_ok=True)

        compile_result = subprocess.run(
            [
                "cc",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-Wno-unused-parameter",
                "-I",
                str(ROOT / "tests" / "fakes"),
                str(ROOT / "tests" / "display_mux_harness.c"),
                "-o",
                str(binary),
            ],
            cwd=ROOT,
            text=True,
            capture_output=True,
        )

        self.assertEqual(compile_result.returncode, 0, compile_result.stderr)

        run_result = subprocess.run(
            [str(binary)],
            cwd=ROOT,
            text=True,
            capture_output=True,
        )

        self.assertEqual(run_result.returncode, 0, run_result.stderr)


if __name__ == "__main__":
    unittest.main()
