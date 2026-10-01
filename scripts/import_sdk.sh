#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 /path/to/RemoteCli-or-SimpleCli-or-CrSDK-root"
  exit 1
fi

SRC="${1%/}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST_INCLUDE="$ROOT/backend/vendor/sony/include"
DEST_LIB="$ROOT/backend/vendor/sony/lib"

mkdir -p "$DEST_INCLUDE" "$DEST_LIB/CrAdapter"

if [[ -d "$SRC/app/CRSDK" && -d "$SRC/external/crsdk" ]]; then
  echo "Importing SDK from sample tree: $SRC"
  rsync -a --delete "$SRC/app/CRSDK/" "$DEST_INCLUDE/"
  rsync -a --delete "$SRC/external/crsdk/" "$DEST_LIB/"
elif [[ -d "$SRC/CrSDK/include" && -d "$SRC/CrSDK/lib" ]]; then
  echo "Importing SDK from package root: $SRC"
  rsync -a --delete "$SRC/CrSDK/include/" "$DEST_INCLUDE/"
  rsync -a --delete "$SRC/CrSDK/lib/" "$DEST_LIB/"
else
  echo "Could not detect a supported SDK layout under: $SRC"
  echo "Expected either:"
  echo "  - app/CRSDK and external/crsdk"
  echo "  - CrSDK/include and CrSDK/lib"
  exit 1
fi

echo "Imported Sony SDK into backend/vendor/sony"
