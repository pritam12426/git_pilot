# GitPilot — Git MCP Server

GitPilot is a **C++20 MCP server** that exposes Git operations as callable tools
for AI assistants. Send it a JSON-RPC request over stdin; it replies with git
data as JSON.

---

## Features

| Area                                        | Status  |
| ------------------------------------------- | ------- |
| Git status, log, diff, commit               | Done    |
| Tool auto-discovery (`tools/list`)          | Done    |
| JSON-RPC 2.0 over stdin (newline-delimited) | Done    |
| Thread-safe logger (stderr / file)          | Done    |
| CLI (mode, log-level, log-file, host, port) | Done    |
| HTTP mode (TCP acceptor skeleton)           | Partial |
| Git clone, add, checkout, merge, show       | Not yet |
| MCP lifecycle handshake (`initialize`)      | Not yet |

---

## Quick Start

```sh
cmake --preset debug
cmake --build --preset debug

# List available tools
echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' \
  | build/debug/src/git_pilot_server stdio

# Show git status
echo '{"jsonrpc":"2.0","method":"tools/call","params":'
'{"name":"status","arguments":{"repo_path":"."}},"id":2}' \
  | build/debug/src/git_pilot_server stdio

# Show recent commits
echo '{"jsonrpc":"2.0","method":"tools/call","params":'
'{"name":"log","arguments":{"repo_path":".","max_count":5}},"id":3}' \
  | build/debug/src/git_pilot_server stdio

# Diff working tree
echo '{"jsonrpc":"2.0","method":"tools/call","params":'
'{"name":"diff","arguments":{"repo_path":"."}},"id":4}' \
  | build/debug/src/git_pilot_server stdio

# Create a commit
echo '{"jsonrpc":"2.0","method":"tools/call","params":'
'{"name":"commit","arguments":{"repo_path":".",'
'"message":"feat: add feature","author_name":"You",'
'"author_email":"you@example.com"}},"id":5}' \
  | build/debug/src/git_pilot_server stdio
```

---

## CLI Usage

```
Usage: git_pilot_server [mode] [options]
```

| Flag               | Default   | Description                                  |
| ------------------ | --------- | -------------------------------------------- |
| `--mode`           | `stdio`   | Server mode: `stdio` or `http`               |
| `-L` `--log-level` | `info`    | Level: off/fatal/error/warn/info/debug/trace |
| `-F` `--log-file`  | (stderr)  | Write logs to file                           |
| `-H` `--host`      | `0.0.0.0` | HTTP bind address                            |
| `-P` `--port`      | `8080`    | HTTP bind port                               |
| `-?` `--help`      |           | Show help                                    |
| `-V` `--version`   |           | Show version                                 |

---

## Build

Requires CMake 4.4.0+, a C++20 compiler, Boost ≥ 1.87.0, and libgit2.

| Preset          | Generator | Binary dir            |
| --------------- | --------- | --------------------- |
| `debug`         | Ninja     | `build/debug`         |
| `release`       | Ninja     | `build/release`       |
| `xcode-debug`   | Xcode     | `build/xcode-debug`   |
| `xcode-release` | Xcode     | `build/xcode-release` |

```sh
cmake --preset debug           # configure
cmake --build --preset debug   # build
ctest --preset all             # (no tests yet)
```

---

## Platform Support

- **macOS** (AppleClang, arm64/x86_64) — primary dev platform
- **Windows** (Visual Studio 17 2022) — CMake presets exist, untested
- **Linux** — not tested but should work with GCC 13+ / Clang 16+

---

## License

MIT — see [LICENSE](LICENSE).

For developers: [DEV.md](DEV.md) | For implementation details: [DEV_IN_DEPTH.md](DEV_IN_DEPTH.md)
