#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
EXAMPLE_DIR="$PROJECT_DIR/examples/watch_dir"
VIBEGRAM="$PROJECT_DIR/build/vibegram"

# Build if needed
if [ ! -x "$VIBEGRAM" ]; then
    "$SCRIPT_DIR/build.sh"
fi

echo "=== vibegram demo ==="
echo "Watching: $EXAMPLE_DIR"
echo "Press Ctrl-C to stop."
echo ""

# Start vibegram in the background, watching the example directory
"$VIBEGRAM" "$EXAMPLE_DIR" &
VIBEGRAM_PID=$!

cleanup() {
    kill "$VIBEGRAM_PID" 2>/dev/null || true
    wait "$VIBEGRAM_PID" 2>/dev/null || true
}
trap cleanup EXIT

# Give the watcher a moment to initialize
sleep 1

# Touch all .vibe files to trigger processing
for f in "$EXAMPLE_DIR"/*.vibe; do
    [ -f "$f" ] || continue
    echo "Triggering: $f"
    touch "$f"
    sleep 0.5
done

# Wait for vibegram to finish processing, then let the user Ctrl-C
wait "$VIBEGRAM_PID"
