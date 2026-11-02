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

Acceptance criterion: eight cases pass, covering stdout-first and stderr-first
262144-byte streams containing every byte value, each with child exit 0 and 7,
under optimized NDEBUG C++17 and strict C++20. The expected presentation is
`Stdout:\n<payload>\n` and `Stderr:\n<payload>\n`, with the existing status diagnostic
on nonzero child exit. Stream order relative to each other is intentionally not
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
