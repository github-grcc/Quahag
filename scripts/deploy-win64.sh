#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

BUILD_DIR="$PROJECT_DIR/build-win"
DEPLOY_DIR="$PROJECT_DIR/deploy/win64"
WINDEPLOYQT="${WINDEPLOYQT_PATH:-$HOME/Qt/6.10.2/mingw_64/bin/windeployqt6.exe}"

if [ ! -f "$BUILD_DIR/Quahag.exe" ]; then
    echo "Error: build-win/Quahag.exe not found. Run scripts/build-win64.sh first."
    exit 1
fi

echo "==> Cleaning deploy directory..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR"

echo "==> Copying Quahag.exe..."
cp "$BUILD_DIR/Quahag.exe" "$DEPLOY_DIR/"

echo "==> Deploying Qt DLLs via windeployqt..."
wine "$WINDEPLOYQT" "$DEPLOY_DIR/Quahag.exe" 2>/dev/null

echo ""
echo "==> Done: $DEPLOY_DIR/"
echo "Files:"
du -sh "$DEPLOY_DIR"/
ls -1 "$DEPLOY_DIR"/
