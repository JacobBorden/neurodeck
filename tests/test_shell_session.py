"""Exercise the real ordinary-user shell, including exit and EOF sessions."""
import os
import pathlib
import subprocess
import shutil
import sys
import tempfile
import unittest

SHELL = pathlib.Path(sys.argv.pop(1)).resolve()


class ShellSessionTest(unittest.TestCase):
    def session(self, text, cwd, env=None):
        result = subprocess.run([str(SHELL)], input=text, text=True,
                                capture_output=True, cwd=cwd, env=env, timeout=10)
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

    def test_plugin_rejection_and_session_recovery(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / 'broken.lua').write_text('this is not valid Lua !!!')
            result = self.session(
                'loadplugin\nunloadplugin\nloadplugin unsupported.txt\n'
                'loadplugin ./missing.so\nloadplugin ./broken.lua\n'
                'unloadplugin ./missing.so\nhelp\n'
                'exec /bin/echo recovery-marker\nexit\n', directory)
        for diagnostic in (
                'Usage: loadplugin <path_to_plugin_file>',
                'Usage: unloadplugin <path_to_plugin_file>',
                'Unsupported plugin type: unsupported.txt',
                'Cannot open C++ library ./missing.so:',
                'Failed to load Lua plugin ./broken.lua:',
                "Plugin from path './missing.so' not found."):
            self.assertIn(diagnostic, result.stderr)
        self.assertIn('Stdout:\nrecovery-marker\n', result.stdout)
        self.assertNotIn('loaded and initialized.', result.stdout)

    def test_lua_plugin_lifecycle(self):
        # Embedded Lua does not require a standalone lua interpreter.
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            shutil.copyfile(pathlib.Path(__file__).resolve().parents[1] /
                            'scripts/plugins/hello.lua', root / 'hello.lua')
            result = self.session(
                'loadplugin ./hello.lua\nhello_lua\n'
                'loadplugin ./hello.lua\nunloadplugin ./hello.lua\n'
                'hello_lua\nloadplugin ./hello.lua\nhello_lua\nexit\n',
                directory)
        self.assertEqual(result.stdout.count('Hello from Lua plugin!\n'), 2)
        self.assertEqual(result.stdout.count("loaded and initialized."), 2)
        self.assertIn("Unknown command: hello_lua.", result.stdout)
        self.assertIn("Calling shutdown for plugin 'HelloLuaPlugin' during "
                      "CommandRegistry destruction.", result.stdout)
        self.assertEqual(result.stderr,
                         "Plugin (C++ or Lua) from path './hello.lua' already loaded.\n")

    def test_missing_external_lua_preserves_embedded_lua_and_session(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / 'empty-path').mkdir()
            (root / 'direct.lua').write_text("print('must-not-run-direct-lua')")
            shutil.copyfile(pathlib.Path(__file__).resolve().parents[1] /
                            'scripts/plugins/hello.lua', root / 'hello.lua')
            environment = os.environ.copy()
            environment['PATH'] = str(root / 'empty-path')
            result = self.session(
                './direct.lua\nloadplugin ./hello.lua\nhello_lua\n'
                'exec /bin/echo after-missing-lua\nexit\n', directory,
                env=environment)
        self.assertIn('Error executing Lua script: ./direct.lua', result.stderr)
        self.assertNotIn('must-not-run-direct-lua', result.stdout)
        self.assertIn('Hello from Lua plugin!\n', result.stdout)
        self.assertIn('Stdout:\nafter-missing-lua\n', result.stdout)
        self.assertIn("Calling shutdown for plugin 'HelloLuaPlugin' during "
                      "CommandRegistry destruction.", result.stdout)

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
