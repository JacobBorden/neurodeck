"""Build, test and classify changed executor coverage with GCC."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


EXECUTOR = Path("shell/commands/exec_command.cpp")
COVERAGE_NOTES = Path("shell/CMakeFiles/shell.dir/commands/exec_command.cpp.gcno")
HUNK_HEADER = re.compile(r"^@@ .*? \+(\d+)(?:,(\d+))? @@", re.MULTILINE)


def git(*args):
    return subprocess.check_output(["git", *args], text=True).strip()


def require_clean_head(head):
    if git("rev-parse", "HEAD") != head or git("status", "--porcelain", "--untracked-files=no"):
        raise ValueError("coverage evidence requires an unchanged HEAD and clean tracked files")


def collect(notes, destination, *, baseline):
    data = notes.with_suffix(".gcda")
    if data.exists() == baseline:
        expected = "absent before tests" if baseline else "present after tests"
        raise ValueError(f"expected executor counters to be {expected}: {data}")

    result = subprocess.run(
        ["gcov", "-b", "-j", "-t", "-o", str(notes), str(EXECUTOR)],
        check=True,
        capture_output=True,
        text=True,
    )
    expected_warning = f"{data}:cannot open data file, assuming not executed" if baseline else ""
    if result.stderr.strip() != expected_warning:
        raise ValueError(f"unexpected gcov diagnostics: {result.stderr.strip()}")
    destination.write_text(result.stdout)
    return json.loads(result.stdout)


def executor_lines(report, source):
    directory = Path(report["current_working_directory"])
    matches = [
        entry for entry in report["files"]
        if (directory / entry["file"]).resolve() == source
    ]
    if len(matches) != 1:
        raise ValueError("expected one executor coverage record from this worktree")
    records = matches[0]["lines"]
    lines = {line["line_number"]: line for line in records}
    if not lines or len(lines) != len(records):
        raise ValueError("expected nonempty, unique executor line records")
    return lines


def coverage_layout(lines):
    return {
        number: (
            line.get("function_name"),
            [{key: value for key, value in branch.items() if key != "count"}
             for branch in line["branches"]],
        )
        for number, line in lines.items()
    }


def report_coverage(baseline, measured, changed, source):
    if coverage_layout(baseline) != coverage_layout(measured):
        raise ValueError("executor coverage records differ from the clean build layout")
    measured_changed = changed.intersection(measured)
    if not measured_changed:
        raise ValueError("no changed executor lines were present in the coverage record")

    missed_lines = partial_lines = 0
    for number in sorted(measured_changed):
        line = measured[number]
        branches = line["branches"]
        uncovered = [branch for branch in branches if branch["count"] == 0]
        if line["count"] == 0:
            missed_lines += 1
        elif uncovered:
            partial_lines += 1
        else:
            continue
        throws = sum(branch["throw"] for branch in uncovered)
        print(
            f"{number}: hits={line['count']}, "
            f"uncovered={len(uncovered)}/{len(branches)} "
            f"(throw={throws}, other={len(uncovered) - throws})  "
            f"{source[number - 1].strip()}"
        )
    print(f"{len(measured_changed)} measured changed lines, {missed_lines} missed, {partial_lines} partial")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base", help="canonical development commit used for the diff")
    parser.add_argument("build", type=Path, help="new directory for build and coverage evidence")
    parser.add_argument("--googletest-source", type=Path, help="local GoogleTest 1.12.1 source")
    args = parser.parse_args()

    try:
        if Path.cwd() != Path(git("rev-parse", "--show-toplevel")).resolve():
            raise ValueError("run from the root of the task worktree")
        head = git("rev-parse", "HEAD")
        require_clean_head(head)
        base = git("rev-parse", "--verify", "--end-of-options", f"{args.base}^{{commit}}")
        subprocess.run(["git", "merge-base", "--is-ancestor", base, head], check=True)
        diff = git("diff", "--no-ext-diff", "--unified=0", base, head, "--", str(EXECUTOR))
        changed = set()
        for start, length in HUNK_HEADER.findall(diff):
            changed.update(range(int(start), int(start) + int(length or 1)))
        if not changed:
            raise ValueError("the selected diff has no added executor lines")

        build = args.build.resolve()
        build.mkdir()
        configure = [
            "cmake", "-S", ".", "-B", str(build), "-G", "Ninja",
            "-DCMAKE_CXX_COMPILER=g++", "-DCMAKE_BUILD_TYPE=Debug", "-DENABLE_COVERAGE=ON",
        ]
        if args.googletest_source:
            configure.append(f"-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST={args.googletest_source.resolve()}")

        subprocess.run(configure, check=True)
        subprocess.run(["cmake", "--build", str(build), "--parallel", "8"], check=True)
        notes = build / COVERAGE_NOTES
        # Build-time GoogleTest discovery can itself write executor counters.
        notes.with_suffix(".gcda").unlink(missing_ok=True)
        baseline = collect(notes, build / "exec-baseline.json", baseline=True)
        subprocess.run(
            ["ctest", "--test-dir", str(build), "--output-on-failure", "--no-tests=error", "-j", "4"],
            check=True,
        )
        measured_path = build / "exec-gcov.json"
        measured = collect(notes, measured_path, baseline=False)
        require_clean_head(head)

        source = EXECUTOR.resolve()
        report_coverage(
            executor_lines(baseline, source), executor_lines(measured, source),
            changed, source.read_text().splitlines(),
        )
        print(f"Base {base}, head {head}, GCC {measured['gcc_version']}")
        print(f"SHA-256 {hashlib.sha256(measured_path.read_bytes()).hexdigest()}  {measured_path}")
    except (OSError, ValueError) as error:
        parser.error(str(error))
    except subprocess.CalledProcessError as error:
        parser.exit(1, f"command failed with exit {error.returncode}: {error.cmd}\n")


if __name__ == "__main__":
    main()
