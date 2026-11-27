# Production mixed-pipe acceptance packet

This bounded packet qualifies the finite output behavior of the production
`ExecCommand`, rather than a copied executor. Run from the task worktree:

```sh
python3 characterization/mixed-pipe-acceptance-v1/run.py
```

Requires Linux/POSIX, Python 3 and a C++17/C++20 compiler (`CXX`, default g++).
The runner resolves the source relative to itself, builds in a temporary directory,
and raises an error on compilation failure, timeout, byte mismatch or an unreaped
child. Each owned probe session is terminated during cleanup, including on timeout.

Acceptance criterion: 24 cases pass, covering stdout-first and stderr-first
262144-byte streams containing every byte value, with both streams populated
or either stream empty, each with child exit 0 and 7,
under optimized NDEBUG C++17 and strict C++20. The expected presentation is
`Stdout:\n<payload>\n` and `Stderr:\n<payload>\n`, with the existing status diagnostic
on nonzero child exit. Empty streams emit no heading or trailing newline; a nonzero exit still emits
the status diagnostic when stderr is empty. Stream order relative to each other is intentionally not
asserted. The void command API does not return the child's exit status; probe exit
zero instead confirms no waitable child remains after execution. Python explicit
checks remain active under optimization.

## Source and compatibility boundaries

Canonical base fetched for this run: development
`fef6996cae551092587ec0434d29f86138665bd6`. Persistent repair branch:
`codex/exec-output-reliability`, preceding head
`ca95d0d251e6ad70bf1ed1b614b62cd957365579`, draft PR #12.
The base is already an ancestor; no integration conflict or history rewrite is
needed. This packet adds no execution policy or production implementation change.

The probe has no Lua dependency. Passing it establishes neither Lua-present
shell/plugin dispatch nor Lua-absent full-build support. Historical Lua-disabled
build failure (undeclared Lua callbacks, exit 2, zero tests) remains unqualified;
the current full Lua-present build is recorded separately in the run evidence.
Native plugin skips do not count as plugin acceptance.

This is trusted execution of a Python child through execvp. Lua/native plugins
and shell commands remain trusted. Neurodeck remains an optional ordinary-user
workbench with conventional session and fallback login; no sandbox, default-shell
replacement, privileged agent or host security change is involved.

Unqualified: dup2 failures and standard-descriptor aliases, allocation/diagnostic
exceptions after fork, parent cancellation/termination policy, bounded capture,
threaded-host fork safety, actual kernel resource exhaustion, terminal read-error
partial capture, Windows, Lua/plugin lifecycle and general shutdown. Existing
proposal decisions concerning SIGPIPE, errno representation and launch channels
remain open. Full CI/static analysis/80% coverage and independent exact-head
review remain separate gates. Revert this packet alone to remove the harness;
executor repair history remains intact.

## Executor patch coverage

Use this check to distinguish untested ordinary branches from unhit GCC
exception edges. Run it from a task worktree with clean tracked files.
It requires GCC with matching gcov, CMake, Ninja and Python 3.

```sh
python3 characterization/mixed-pipe-acceptance-v1/report_exec_coverage.py \
  fef6996cae551092587ec0434d29f86138665bd6 work/exec-coverage
```

Use a new build directory whose parent already exists. The runner configures
and builds with coverage, records the executor line and branch layout before
CTest, and collects counters after CTest succeeds. It rejects missing counters
or a report whose layout differs from the baseline. The build directory
retains both JSON files. The summary records the exact base, head, GCC version
and measured JSON SHA-256. Use `--googletest-source` with a local copy of the
project's pinned GoogleTest 1.12.1 source when a fetch is unavailable.

On PR #12 head `5df94839374b95898dc8e9d12433886385c36d66`, GCC 15.2.1
reported 46 measured changed lines, zero missed lines and ten partial lines.
All uncovered branches on those lines were marked `throw` in gcov JSON.
They occur at diagnostics, POSIX calls and the capture string append.
Counting each partial line as uncovered gives 36/46 (78.26%), matching the
[PR patch report](https://app.codecov.io/gh/JacobBorden/neurodeck/pull/12).
The GCC classification is based on local data. The Codecov report independently
shows ten partials and a failed 100% patch check.

The report is diagnostic. The project-wide 80% line gate and Codecov patch
check still apply. Post-fork exception handling needs a defined cleanup,
reaping and diagnostic contract before treating these edges as acceptance
cases. The coverage build uses Lua 5.4. Full Lua-disabled build support
remains unqualified.
