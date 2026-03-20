# Repository Guidelines

## Project Structure & Module Organization
`include/vibegram/` contains public headers. Reusable implementation lives in `src/lib/`, and the CLI entry point lives in `app/vibegram/`. Build configuration is in `CMakeLists.txt` and `cmake/`. Use `examples/watch_dir/` for local manual testing inputs. Treat `build/` as generated output; do not hand-edit files there.

## Build, Test, and Development Commands
Configure from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Run the binary against the current working directory or a custom directory:

```bash
./build/vibegram
./build/vibegram /path/to/project
```

Install or remove the binary with CMake targets:

```bash
cmake --build build --target install
cmake --build build --target uninstall
```

Run tests with:

```bash
ctest --test-dir build --output-on-failure
```

This project fetches `efsw`, `nlohmann/json`, and `spdlog` during configure, and requires a system `libcurl`.

## Coding Style & Naming Conventions
Use C++20 and match the existing style: 4-space indentation, opening braces on the same line, and concise `//` comments only where behavior is not obvious. Keep public headers under `include/vibegram/`, library implementation under `src/lib/`, and executable-only code under `app/vibegram/`. Use `snake_case` for functions and variables, `PascalCase` for types, and keep code inside the `vibegram` namespace.

## Testing Guidelines
The repository uses CTest. Run `ctest --test-dir build --output-on-failure` after rebuilding. Keep tests in `tests/` with names like `llm_client_test.cpp` or `watcher_test.cpp`. Prefer deterministic tests that do not require live network calls. Watcher tests should verify that only `.vibe` files are accepted.

## Commit & Pull Request Guidelines
The repository has no commit history yet, so use short imperative commit subjects such as `Add retry handling for API errors`. Keep commits scoped to one change. Pull requests should include a brief summary, manual verification steps, any new environment variables, and screenshots or logs when behavior changes are user-visible.

## Security & Configuration Tips
Set `ANTHROPIC_API_KEY` in your shell before running the app. Never commit API keys, generated build artifacts, or local watch-directory contents that may contain sensitive prompts or responses.
