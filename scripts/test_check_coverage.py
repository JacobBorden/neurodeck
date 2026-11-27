"""CLI regressions for the required per-directory coverage gate."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).with_name('check_coverage.py')


def record(name, hits):
    return f'SF:{name}\n' + ''.join(
        f'DA:{line},{hit}\n' for line, hit in enumerate(hits, 1)) + 'end_of_record\n'


class CoverageGateTests(unittest.TestCase):
    def check_report(self, report, expected, *messages):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'coverage.info'
            path.write_text(report)
            result = subprocess.run(
                [sys.executable, str(SCRIPT), str(path), '80', 'core/', 'shell/'],
                capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, expected, result.stdout + result.stderr)
        for message in messages:
            self.assertIn(message, result.stdout)

    def test_missing_core_fails_even_with_full_shell(self):
        self.check_report(record('shell/command.cpp', [1]), 1,
                          'No executable line coverage data for core/')

    def test_missing_shell_fails_even_with_full_core(self):
        self.check_report(record('core/file_io.cpp', [1]), 1,
                          'No executable line coverage data for shell/')

    def test_zero_line_core_fails(self):
        self.check_report(record('core/file_io.cpp', []) +
                          record('shell/command.cpp', [1]), 1,
                          'No executable line coverage data for core/')

    def test_empty_report_fails(self):
        self.check_report('', 1, 'No executable line coverage data for core/',
                          'No executable line coverage data for shell/')

    def test_below_threshold_fails(self):
        self.check_report(record('core/file_io.cpp', [1, 0]) +
                          record('shell/command.cpp', [1]), 1, 'below the required')

    def test_each_directory_meets_threshold(self):
        self.check_report(record('core/file_io.cpp', [1, 1, 1, 1, 0]) +
                          record('shell/command.cpp', [1]), 0,
                          'Coverage for core/: 80.00%', 'Coverage for shell/: 100.00%')


if __name__ == '__main__':
    unittest.main()
