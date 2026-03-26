#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/release}"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
PACKAGE_ARCH="${PACKAGE_ARCH:-$(uname -m)}"
PACKAGE_NAME="leap-and-land-linux-${PACKAGE_ARCH}"
APP_DIR="$DIST_DIR/$PACKAGE_NAME"
LIB_DIR="$APP_DIR/lib"
BINARY_PATH="$BUILD_DIR/src/Leap_And_Land"
APP_BINARY="$APP_DIR/Leap_And_Land"
ARCHIVE_PATH="$DIST_DIR/$PACKAGE_NAME.tar.gz"

should_bundle() {
    local lib_name
    lib_name="$(basename "$1")"

    case "$lib_name" in
        ld-linux*.so*|libc.so*|libm.so*|libpthread.so*|libdl.so*|librt.so*|libresolv.so*)
            return 1
            ;;
        *)
            return 0
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
mkdir -p "$APP_DIR" "$LIB_DIR"

cp "$BINARY_PATH" "$APP_BINARY"
strip "$APP_BINARY" || true

if command -v patchelf >/dev/null 2>&1; then
    patchelf --set-rpath '$ORIGIN/lib' "$APP_BINARY"
fi

cp -R "$ROOT_DIR/res" "$APP_DIR/"
cp "$ROOT_DIR/README.md" "$ROOT_DIR/LICENSE" "$APP_DIR/"

cat > "$APP_DIR/run.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="$APP_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

cd "$APP_DIR"
exec "$APP_DIR/Leap_And_Land" "$@"
EOF
chmod +x "$APP_DIR/run.sh"

echo "==> Bundling shared libraries"
mapfile -t dependencies < <(
    ldd "$BINARY_PATH" | awk '
        /=> \// { print $3 }
        /^\/.*\.so/ { print $1 }
    ' | sort -u
)

for dependency in "${dependencies[@]}"; do
    if should_bundle "$dependency"; then
        cp -L "$dependency" "$LIB_DIR/$(basename "$dependency")"
    fi
done

echo "==> Creating archive"
mkdir -p "$DIST_DIR"
rm -f "$ARCHIVE_PATH"
tar -C "$DIST_DIR" -czf "$ARCHIVE_PATH" "$PACKAGE_NAME"

echo
echo "Package ready:"
echo "  Directory: $APP_DIR"
echo "  Archive:   $ARCHIVE_PATH"
