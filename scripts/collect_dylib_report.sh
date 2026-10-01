#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_FILE="${1:-$ROOT/dist/dylib-report.txt}"
mkdir -p "$(dirname "$OUT_FILE")"

{
  echo "# dylib report"
  echo
  for target in \
    "$ROOT/backend/build/fx6d" \
    "$ROOT/backend/build/libCr_Core.dylib" \
    "$ROOT/backend/build/libmonitor_protocol.dylib" \
    "$ROOT/backend/build/libmonitor_protocol_pf.dylib" \
    "$ROOT/backend/build/Contents/Frameworks/CrAdapter/libCr_PTP_IP.dylib" \
    "$ROOT/backend/build/Contents/Frameworks/CrAdapter/libCr_PTP_USB.dylib" \
    "$ROOT/backend/build/Contents/Frameworks/CrAdapter/libssh2.dylib" \
    "$ROOT/backend/build/Contents/Frameworks/CrAdapter/libusb-1.0.0.dylib"
  do
    echo "## $target"
    file "$target"
    otool -L "$target"
    echo
  done
} >"$OUT_FILE"

echo "wrote $OUT_FILE"
