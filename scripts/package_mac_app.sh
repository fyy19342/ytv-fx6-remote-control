#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 /path/to/FX6OperationApp.app /path/to/backend-build-or-fx6d"
  exit 1
fi

APP_PATH="$1"
BACKEND_INPUT="$2"

if [ ! -d "$APP_PATH" ]; then
  echo "App bundle not found: $APP_PATH"
  exit 1
fi

if [ ! -e "$BACKEND_INPUT" ]; then
  echo "Backend input not found: $BACKEND_INPUT"
  exit 1
fi

if [ -d "$BACKEND_INPUT" ]; then
  BACKEND_DIR="$BACKEND_INPUT"
else
  BACKEND_DIR="$(cd "$(dirname "$BACKEND_INPUT")" && pwd)"
fi

TARGET_ROOT="$APP_PATH/Contents/Resources/backend"
TARGET_DIR="$TARGET_ROOT/build"
rm -rf "$TARGET_DIR"
mkdir -p "$TARGET_ROOT"
mkdir -p "$TARGET_DIR"
cp "$BACKEND_DIR/fx6d" "$BACKEND_DIR/"*.dylib "$TARGET_DIR/"
cp -R "$BACKEND_DIR/Contents" "$TARGET_DIR/"
cp "$ROOT/build_info.json" "$TARGET_DIR/build_info.json"
chmod +x "$TARGET_DIR/fx6d"
cp "$ROOT/build_info.json" "$APP_PATH/Contents/Resources/build_info.json"
rm -rf "$APP_PATH/Contents/Resources/licenses"
cp -R "$ROOT/licenses" "$APP_PATH/Contents/Resources/licenses"
cp "$ROOT/THIRD_PARTY_NOTICES.md" "$APP_PATH/Contents/Resources/THIRD_PARTY_NOTICES.md"
cp "$ROOT/docs/TERMS.md" "$APP_PATH/Contents/Resources/TERMS.md"

codesign --force --sign - "$TARGET_DIR/fx6d"
codesign --force --deep --sign - "$APP_PATH"

echo "Backend runtime copied to:"
echo "  $TARGET_DIR"
