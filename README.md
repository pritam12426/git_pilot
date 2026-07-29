# GitPilot — Git MCP Server

**GitPilot** is a C++20 server that implements the [Model Context Protocol][mcp]
(MCP) so AI assistants can call Git operations through a standardized tool
interface.

## Status

Under active development. The logging system, tool abstraction layer, and build
pipeline are implemented. The MCP protocol, transport layer, and concrete Git
tool implementations are planned. See [DEV.md](DEV.md) for current capabilities.

## Features

| Area                                          | Status  |
| --------------------------------------------- | ------- |
| Thread-safe logger (stderr / file)            | Done    |
| Tool abstraction interface                    | Done    |
| MCP protocol (JSON-RPC 2.0)                   | Planned |
| stdio transport                               | Planned |
| HTTP transport                                | Planned |
| Git operations (clone/status/commit/log/diff) | Planned |

## Quick Start

```sh
cmake --preset debug                        # configure (Ninja, build/debug)
cmake --build --preset debug                # build
build/debug/src/git_pilot_server            # run (logs to stderr, then exits)
```

Requirements: CMake 4.4.0+, C++20 compiler, Boost >= 1.87.0, libgit2.

## Build Presets

| Preset          | Generator | Binary dir            |
| --------------- | --------- | --------------------- |
| `debug`         | Ninja     | `build/debug`         |
| `release`       | Ninja     | `build/release`       |
| `xcode-debug`   | Xcode     | `build/xcode-debug`   |
| `xcode-release` | Xcode     | `build/xcode-release` |

## License

MIT — see [LICENSE](LICENSE).

[mcp]: https://modelcontextprotocol.io
