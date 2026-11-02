"""Compile the current production executor; verify finite trusted child output."""
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent


def run_child(command):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               start_new_session=True)
    try:
        return (*process.communicate(timeout=10), process.returncode)
    finally:
        # The session is owned by this probe; also stop descendants on failure.
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        process.wait()


def main():
    passed = 0
    with tempfile.TemporaryDirectory(prefix="neurodeck-mixed-pipe-") as directory:
        for standard in ("c++17", "c++20"):
            binary = str(Path(directory) / standard)
            command = [os.environ.get("CXX", "g++"), f"-std={standard}",
                       "-pedantic-errors", "-Wall", "-Wextra", "-O2", "-DNDEBUG",
                       "-I", str(ROOT / "shell/commands"), str(HERE / "probe.cpp"),
                       str(ROOT / "shell/commands/exec_command.cpp"), "-o", binary]
            subprocess.run(command, check=True, timeout=60)
            for first in (1, 2):
                for code in (0, 7):
                    # Both streams exceed ordinary pipe capacity and contain NULs.
                    out = bytes(range(256)) * 1024
                    err = bytes(reversed(range(256))) * 1024
                    script = (
                        "import os,sys\n"
                        "payloads={1:bytes(range(256))*1024,"
                        "2:bytes(reversed(range(256)))*1024}\n"
                        f"for fd in ({first},{3-first}):\n"
                        " data=payloads[fd]\n"
                        " while data:\n"
                        "  n=os.write(fd,data); data=data[n:]\n"
                        f"sys.exit({code})\n"
                    )
                    actual_out, actual_err, result = run_child(
                        [binary, sys.executable, "-c", script])
                    expected_err = b"Stderr:\n" + err + b"\n"
                    if code:
                        expected_err += f"Command exited with status {code}\n".encode()
                    if (result != 0 or actual_out != b"Stdout:\n" + out + b"\n"
                            or actual_err != expected_err):
                        raise RuntimeError(f"{standard} first={first} exit={code}: "
                                           f"probe={result}, bytes="
                                           f"{len(actual_out)}/{len(actual_err)}")
                    passed += 1
                    print(f"PASS {standard} first={first} exit={code}")
    print(f"{passed}/{passed} passed")


if __name__ == "__main__":
    main()
