#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/release-windows}"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
PACKAGE_ARCH="${PACKAGE_ARCH:-${MSYSTEM_CARCH:-x86_64}}"
PACKAGE_NAME="leap-and-land-windows-${PACKAGE_ARCH}"
APP_DIR="$DIST_DIR/$PACKAGE_NAME"
BINARY_PATH="$BUILD_DIR/src/Leap_And_Land.exe"
APP_BINARY="$APP_DIR/Leap_And_Land.exe"
ARCHIVE_PATH="$DIST_DIR/$PACKAGE_NAME.zip"

should_bundle() {
    case "$1" in
        /mingw*/bin/*|/ucrt*/bin/*|/clang*/bin/*)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

cmake_args=(-S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release)
if [[ -n "${CMAKE_GENERATOR:-}" ]]; then
    cmake_args+=(-G "$CMAKE_GENERATOR")
fi

echo "==> Configuring Release build"
cmake "${cmake_args[@]}"

echo "==> Building release binary"
cmake --build "$BUILD_DIR" --config Release

if [[ ! -f "$BINARY_PATH" ]]; then
    echo "Release binary not found at $BINARY_PATH" >&2
    exit 1
fi

echo "==> Preparing package directory"
rm -rf "$APP_DIR"
mkdir -p "$APP_DIR"

cp "$BINARY_PATH" "$APP_BINARY"
cp -R "$ROOT_DIR/res" "$APP_DIR/"
cp "$ROOT_DIR/README.md" "$ROOT_DIR/LICENSE" "$APP_DIR/"

cat > "$APP_DIR/run.bat" <<'EOF'
@echo off
setlocal
cd /d "%~dp0"
"%~dp0Leap_And_Land.exe" %*
EOF

echo "==> Bundling runtime DLLs"
mapfile -t dependencies < <(
    ldd "$BINARY_PATH" | awk '
        /=> \// { print $3 }
        /^\/.*\.dll/ { print $1 }
    ' | sort -u
)

for dependency in "${dependencies[@]}"; do
    if should_bundle "$dependency"; then
        cp -L "$dependency" "$APP_DIR/$(basename "$dependency")"
    fi
done

echo "==> Creating archive"
mkdir -p "$DIST_DIR"
rm -f "$ARCHIVE_PATH"
(
    cd "$DIST_DIR"
    zip -rq "$(basename "$ARCHIVE_PATH")" "$PACKAGE_NAME"
)

echo
echo "Package ready:"
echo "  Directory: $APP_DIR"
echo "  Archive:   $ARCHIVE_PATH"
