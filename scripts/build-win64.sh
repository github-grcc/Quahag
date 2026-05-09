#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

QT_MINGW="/home/grcc/Qt/6.10.2/mingw_64"
QT_HOST="/home/grcc/Qt/6.10.2/gcc_64"
BUILD_DIR="$PROJECT_DIR/build-win"

echo "==> Configuring for Windows x86_64 (Release)..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/cmake/toolchain-win64.cmake" \
    -DCMAKE_PREFIX_PATH="$QT_MINGW" \
    -DQT_HOST_PATH="$QT_HOST"

echo ""
echo "==> Building..."
cmake --build "$BUILD_DIR" --parallel

echo ""
echo "==> Done: $BUILD_DIR/Quahag.exe"
