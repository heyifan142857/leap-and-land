#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/release-macos}"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
PACKAGE_ARCH="${PACKAGE_ARCH:-$(uname -m)}"
PACKAGE_NAME="leap-and-land-macos-${PACKAGE_ARCH}"
APP_DIR="$DIST_DIR/$PACKAGE_NAME"
LIB_DIR="$APP_DIR/lib"
BINARY_PATH="$BUILD_DIR/src/Leap_And_Land"
APP_BINARY="$APP_DIR/Leap_And_Land"
ARCHIVE_PATH="$DIST_DIR/$PACKAGE_NAME.zip"

declare -a MACOS_LIB_SEARCH_DIRS=()

add_search_dir() {
    local dir="$1"
    local existing_dir

    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return
    fi

    for existing_dir in "${MACOS_LIB_SEARCH_DIRS[@]}"; do
        if [[ "$existing_dir" == "$dir" ]]; then
            return
        fi
    done

    MACOS_LIB_SEARCH_DIRS+=("$dir")
}

list_dependency_refs() {
    local file="$1"

    otool -L "$file" | tail -n +2 | awk '{print $1}'
}

list_dylib_id() {
    local file="$1"

    otool -D "$file" 2>/dev/null | tail -n +2 | head -n 1 || true
}

resolve_dependency_path() {
    local dependency_ref="$1"
    local file_dir="$2"
    local lib_name
    local search_dir

    case "$dependency_ref" in
        /System/*|/usr/lib/*)
            return 1
            ;;
        /*)
            if [[ -f "$dependency_ref" ]]; then
                printf '%s\n' "$dependency_ref"
                return 0
            fi
            ;;
        @loader_path/*|@executable_path/*|@rpath/*)
            lib_name="$(basename "$dependency_ref")"

            if [[ -f "$file_dir/$lib_name" ]]; then
                printf '%s\n' "$file_dir/$lib_name"
                return 0
            fi

            if [[ -f "$LIB_DIR/$lib_name" ]]; then
                printf '%s\n' "$LIB_DIR/$lib_name"
                return 0
            fi

            for search_dir in "${MACOS_LIB_SEARCH_DIRS[@]}"; do
                if [[ -f "$search_dir/$lib_name" ]]; then
                    printf '%s\n' "$search_dir/$lib_name"
                    return 0
                fi
            done
            ;;
    esac

    echo "Unable to resolve dependency: $dependency_ref" >&2
    return 1
}

rewrite_dependencies() {
    local file="$1"
    local file_dir
    local dependency_ref
    local resolved_dependency
    local dependency_name
    local copied_dependency
    local replacement_ref
    local self_id

    file_dir="$(dirname "$file")"
    self_id="$(list_dylib_id "$file")"

    while IFS= read -r dependency_ref; do
        if [[ -n "$self_id" && "$dependency_ref" == "$self_id" ]]; then
            continue
        fi

        resolved_dependency="$(resolve_dependency_path "$dependency_ref" "$file_dir")" || continue
        dependency_name="$(basename "$resolved_dependency")"
        copied_dependency="$LIB_DIR/$dependency_name"

        if [[ ! -f "$copied_dependency" ]]; then
            cp -L "$resolved_dependency" "$copied_dependency"
            chmod u+w "$copied_dependency"
            install_name_tool -id "@loader_path/$dependency_name" "$copied_dependency"
            rewrite_dependencies "$copied_dependency"
        fi

        if [[ "$file" == "$APP_BINARY" ]]; then
            replacement_ref="@executable_path/lib/$dependency_name"
        else
            replacement_ref="@loader_path/$dependency_name"
        fi

        install_name_tool -change "$dependency_ref" "$replacement_ref" "$file"
    done < <(list_dependency_refs "$file")
}

cmake_args=(-S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release)
if [[ -n "${CMAKE_GENERATOR:-}" ]]; then
    cmake_args+=(-G "$CMAKE_GENERATOR")
fi

if command -v brew >/dev/null 2>&1; then
    add_search_dir "$(brew --prefix)/lib"
fi

while IFS= read -r lib_dir; do
    add_search_dir "$lib_dir"
done < <(pkg-config --variable=libdir sdl2 SDL2_image SDL2_ttf SDL2_mixer 2>/dev/null | sort -u)

add_search_dir "/usr/local/lib"
add_search_dir "/opt/homebrew/lib"

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
chmod u+w "$APP_BINARY"
cp -R "$ROOT_DIR/res" "$APP_DIR/"
cp "$ROOT_DIR/README.md" "$ROOT_DIR/LICENSE" "$APP_DIR/"

rewrite_dependencies "$APP_BINARY"

if command -v codesign >/dev/null 2>&1; then
    find "$LIB_DIR" -type f -name '*.dylib' -exec codesign --force --sign - {} \;
    codesign --force --sign - "$APP_BINARY"
fi

cat > "$APP_DIR/run.command" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export DYLD_LIBRARY_PATH="$APP_DIR/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"

cd "$APP_DIR"
exec "$APP_DIR/Leap_And_Land" "$@"
EOF
chmod +x "$APP_DIR/run.command"

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
