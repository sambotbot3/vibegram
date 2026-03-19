#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
BUILD_TYPE="${1:-Release}"
JOBS="${2:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

echo "=== vibegram build ==="
echo "Type: $BUILD_TYPE | Jobs: $JOBS"

cmake -B "$BUILD_DIR" -S "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "$BUILD_DIR" -j "$JOBS"

# Symlink compile_commands.json to project root for clangd / IDEs
if [ -f "$BUILD_DIR/compile_commands.json" ]; then
    ln -sf "$BUILD_DIR/compile_commands.json" "$PROJECT_DIR/compile_commands.json"
fi

echo ""
echo "Built: $BUILD_DIR/vibegram"
