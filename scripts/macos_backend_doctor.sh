#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-$ROOT/backend/build/fx6d}"
RUNTIME_DIR="$(cd "$(dirname "$TARGET")" && pwd)"

echo "== file =="
file "$TARGET"
echo

echo "== otool -L fx6d =="
otool -L "$TARGET"
echo

echo "== otool -l fx6d (rpath/id/load) =="
otool -l "$TARGET" | rg "LC_RPATH|path |LC_ID_DYLIB|LC_LOAD_DYLIB" || true
echo

echo "== adapter tree =="
find "$RUNTIME_DIR/Contents/Frameworks/CrAdapter" -maxdepth 1 -type f | sort
echo

echo "== libCr_PTP_IP strings (libssh2/libusb) =="
strings "$RUNTIME_DIR/Contents/Frameworks/CrAdapter/libCr_PTP_IP.dylib" | rg "libssh2|libusb" || true
echo

echo "== codesign =="
codesign -dv --verbose=4 "$TARGET" || true
echo

echo "== spctl =="
spctl --assess --type execute --verbose=4 "$TARGET" || true
