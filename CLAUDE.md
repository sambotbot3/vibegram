# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is vibegram

Vibegram is a plain-English programming language transpiler. It watches a directory for `.vibe` files and sends their contents to an LLM (OpenAI or Anthropic) for processing. The core loop: filesystem watcher detects `.vibe` file changes → reads file → sends to LLM API → prints response.

## Build and run

```bash
./install-deps.sh          # one-time: installs cmake, libcurl-dev, etc.
./scripts/build.sh                 # configure + build (Release by default)
./scripts/build.sh Debug           # debug build
```

Or manually:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Run: `./build/vibegram [/path/to/watch]` (defaults to cwd).

## Tests

```bash
ctest --test-dir build --output-on-failure
```

Tests are plain C++ executables (no framework) that return 0 on success. Test files go in `tests/` with names like `foo_test.cpp` and must be registered in `tests/CMakeLists.txt`.

## Architecture

- **`prompts/transpile.md`** — the system prompt sent to the LLM with every `.vibe` file. Loaded at startup by searching upward from cwd. Editable without recompiling.
- **`app/vibegram/main.cpp`** — CLI entry point. Loads the transpile prompt, sets up signal handling, creates `LLMClient` and `Watcher`, runs the event loop.
- **`vibegram_lib`** — static library linked by both the executable and tests. Contains:
  - `Watcher` (`include/vibegram/watcher.h`, `src/lib/watcher.cpp`) — wraps efsw to watch a directory, filters to `.vibe` files only, invokes a callback on add/modify.
  - `LLMClient` (`include/vibegram/llm_client.h`, `src/lib/llm_client.cpp`) — sends file contents to OpenAI or Anthropic APIs via libcurl. Supports multiple credential sources with this priority: explicit config → Codex OAuth (`~/.codex/auth.json`) → `OPENAI_API_KEY` → `ANTHROPIC_API_KEY`. Currently **only Codex OAuth is accepted**; fallback credentials are rejected at construction.

## Dependencies

Fetched by CMake (FetchContent): `efsw` (file watcher), `nlohmann/json`, `spdlog` (logging). System dependency: `libcurl`.

## Code style

C++20. `snake_case` for functions/variables, `PascalCase` for types. Everything in `vibegram` namespace. 4-space indent, opening braces on same line. Public headers in `include/vibegram/`, library code in `src/lib/`, executables in `app/vibegram/`.
