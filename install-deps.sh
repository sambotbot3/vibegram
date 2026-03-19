#!/usr/bin/env bash
set -euo pipefail

echo "=== vibegram dependency installer ==="

# Detect package manager
if command -v apt-get &>/dev/null; then
    echo "Detected apt (Debian/Ubuntu)"
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        git \
        libcurl4-openssl-dev \
        pkg-config
elif command -v dnf &>/dev/null; then
    echo "Detected dnf (Fedora/RHEL)"
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        git \
        libcurl-devel \
        pkg-config
elif command -v pacman &>/dev/null; then
    echo "Detected pacman (Arch)"
    sudo pacman -S --needed \
        base-devel \
        cmake \
        git \
        curl \
        pkg-config
elif command -v brew &>/dev/null; then
    echo "Detected Homebrew (macOS)"
    brew install cmake curl pkg-config
else
    echo "ERROR: No supported package manager found."
    echo "Install manually: cmake, git, libcurl (dev), pkg-config, C++20 compiler"
    exit 1
fi

echo ""
echo "System deps installed. C++ libraries (efsw, nlohmann/json, spdlog)"
echo "are fetched automatically by CMake at build time."
echo ""
echo "Build with:"
echo "  cmake -B build -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build build -j\$(nproc)"
