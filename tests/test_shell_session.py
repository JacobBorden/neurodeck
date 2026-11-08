"""Exercise the real ordinary-user shell, including exit and EOF sessions."""
import pathlib
import subprocess
import shutil
import sys
import tempfile
import unittest

SHELL = pathlib.Path(sys.argv.pop(1)).resolve()


class ShellSessionTest(unittest.TestCase):
    def session(self, text, cwd):
        result = subprocess.run([str(SHELL)], input=text, text=True,
                                capture_output=True, cwd=cwd, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('Welcome to Neurodeck shell!', result.stdout)
        self.assertIn('Exiting Neurodeck shell. Goodbye!', result.stdout)
        return result

    def test_commands_and_explicit_exit(self):
        with tempfile.TemporaryDirectory() as directory:
            result = self.session('\nunknown_session_command\nhelp\nopen resource\n'
                                  'exec /bin/echo session-marker\nexit\n'
                                  'exec /bin/echo must-not-run\n', directory)
        self.assertIn('Unknown command: unknown_session_command.', result.stdout)
        self.assertIn("Attempting to open 'resource'", result.stdout)
        self.assertIn('Stdout:\nsession-marker\n', result.stdout)
        self.assertNotIn('must-not-run', result.stdout)
        self.assertEqual(result.stderr, '')

    def test_direct_scripts_and_eof(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / 'ok.sh').write_text('#!/bin/sh\necho direct-marker\n')
            (root / 'fail.sh').write_text('#!/bin/sh\nexit 7\n')
            (root / 'ok.sh').chmod(0o700)
            (root / 'fail.sh').chmod(0o700)
            (root / 'plain').write_text('ordinary file')
            result = self.session('./ok.sh\n./fail.sh\n./plain\n', directory)
        self.assertIn('direct-marker', result.stdout)
        self.assertIn('Unknown command: ./plain.', result.stdout)
        self.assertEqual(result.stderr, 'Error executing: ./fail.sh\n')

    @unittest.skipUnless(shutil.which('lua'), 'Lua interpreter absent')
    def test_direct_lua_scripts(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / 'ok.lua').write_text("print('lua-session-marker')")
            (root / 'fail.lua').write_text("error('intentional-session-error')")
            result = self.session('./ok.lua\n./fail.lua\nexit\n', directory)
        self.assertIn('lua-session-marker', result.stdout)
        self.assertIn('intentional-session-error', result.stderr)
        self.assertIn('Error executing Lua script: ./fail.lua', result.stderr)


if __name__ == '__main__':
    unittest.main()
