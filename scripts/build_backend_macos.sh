#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/backend/build"
ABS_VENDOR_RPATH="$ROOT/backend/vendor/sony/lib"

cmake -S "$ROOT/backend" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
cmake --build "$BUILD_DIR" -j4
ctest --test-dir "$BUILD_DIR" --output-on-failure

if otool -l "$BUILD_DIR/fx6d" | grep -Fq "$ABS_VENDOR_RPATH"; then
  install_name_tool -delete_rpath "$ABS_VENDOR_RPATH" "$BUILD_DIR/fx6d"
fi

codesign --force --sign - "$BUILD_DIR/fx6d"
cp "$ROOT/build_info.json" "$BUILD_DIR/build_info.json"

echo "backend ready: $BUILD_DIR/fx6d"
