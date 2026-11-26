"""Classify uncovered changed executor lines and branches in GCC coverage JSON."""

import argparse
import json
from pathlib import Path
import re
import subprocess


EXECUTOR = Path("shell/commands/exec_command.cpp")
HUNK_HEADER = re.compile(r"^@@ .*? \+(\d+)(?:,(\d+))? @@", re.MULTILINE)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base", help="canonical development commit used for the diff")
    parser.add_argument("report", type=Path, help="uncompressed gcov -b -j -t output")
    args = parser.parse_args()

    diff = subprocess.check_output(
        ["git", "diff", "--no-ext-diff", "--unified=0", args.base, "HEAD", "--", str(EXECUTOR)],
        text=True,
    )
    changed = set()
    for start, length in HUNK_HEADER.findall(diff):
        changed.update(range(int(start), int(start) + int(length or 1)))

    files = json.loads(args.report.read_text())["files"]
    matches = [entry for entry in files if Path(entry["file"]).resolve() == EXECUTOR.resolve()]
    if len(matches) != 1:
        parser.error("expected one executor coverage record from this worktree")
    if not changed:
        parser.error("the selected diff has no added executor lines")

    source = EXECUTOR.read_text().splitlines()
    counted_lines = missed_lines = partial_lines = 0
    for line in matches[0]["lines"]:
        number = line["line_number"]
        if number not in changed:
            continue

        counted_lines += 1
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

    if not counted_lines:
        parser.error("no changed executor lines were present in the coverage record")
    print(f"{counted_lines} measured changed lines, {missed_lines} missed, {partial_lines} partial")


if __name__ == "__main__":
    main()
