#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

"$SCRIPT_DIR/build.sh" "$@"

echo ""
echo "=== vibegram tests ==="
ctest --test-dir "$PROJECT_DIR/build" --output-on-failure
