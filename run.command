#!/bin/bash
# ============================================================
#  CubeRender — macOS / Linux double-click launcher
#
#  Double-click this file in Finder  → Terminal opens → cube runs.
#  (If macOS blocks the script: right-click → Open → Open)
#
#  This script compiles cube.c if needed, then runs the program.
# ============================================================

# Move to the directory containing this script
cd "$(dirname "$0")"

BINARY="./cube"
SOURCE="cube.c"

echo "========================================"
echo "  CubeRender v2.0"
echo "========================================"
echo ""

# Check for a C compiler
if command -v cc &>/dev/null; then
    CC=cc
elif command -v gcc &>/dev/null; then
    CC=gcc
elif command -v clang &>/dev/null; then
    CC=clang
else
    echo "[ERROR] No C compiler found."
    echo "Install Xcode Command Line Tools:"
    echo "  xcode-select --install"
    read -rp "Press Enter to close..."
    exit 1
fi

# Compile if binary is missing or source is newer
if [ ! -f "$BINARY" ] || [ "$SOURCE" -nt "$BINARY" ]; then
    echo "[Build] Compiling $SOURCE ..."
    if ! $CC -O2 -Wall -std=c11 -o cube "$SOURCE" -lm; then
        echo ""
        echo "[ERROR] Compilation failed."
        read -rp "Press Enter to close..."
        exit 1
    fi
    echo "[Build] Done."
    echo ""
fi

echo "Starting CubeRender...  Quit: Ctrl+C"
echo ""

$BINARY "$@"

echo ""
echo "CubeRender exited."
read -rp "Press Enter to close this window..."
