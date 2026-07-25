#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

# Build self-contained Qt/KDE app bundle
# Don't hardcode project-specific details here, put it in config.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$(dirname "$SCRIPT_DIR")"
# shellcheck source=config.sh
source "$SCRIPT_DIR/config.sh"

DEPS_DIR="$SOURCE_DIR/deps"
KF6_PREFIX="$DEPS_DIR/kf6-prefix"
BUILD_DIR="$SOURCE_DIR/build"
DIST_DIR="$SOURCE_DIR/dist"
APP_BUNDLE="$DIST_DIR/$APP_NAME.app"

BUILD_TYPE="Release"
JOBS="$(sysctl -n hw.ncpu)"
STEPS="deps app bundle"
SIGN_IDENTITY="-"     # ad-hoc signature by default
MAKE_DMG=0
CLEAN=0

log()  { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
step() { printf '\033[1;32m--> %s\033[0m\n' "$*"; }
die()  { printf '\033[1;31merror:\033[0m %s\n' "$*" >&2; exit 1; }

usage() {
    sed -n '6,17p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
    cat <<EOF

Options:
  --step <deps|app|bundle>   Run only the named step (repeatable)
  --build-type <type>        CMake build type (default: $BUILD_TYPE)
  --jobs <n>                 Parallel build jobs (default: $JOBS)
  --sign <identity>          Codesign identity (default: ad-hoc "-")
  --dmg                      Also produce dist/$APP_NAME.dmg
  --clean                    Remove deps/ build artifacts and build/ first
  --help                     Show this message
EOF
}

##### Parse Arguments
explicit_steps=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --step)       explicit_steps+=("$2"); shift 2 ;;
        --build-type) BUILD_TYPE="$2"; shift 2 ;;
        --jobs)       JOBS="$2"; shift 2 ;;
        --sign)       SIGN_IDENTITY="$2"; shift 2 ;;
        --dmg)        MAKE_DMG=1; shift ;;
        --clean)      CLEAN=1; shift ;;
        --help|-h)    usage; exit 0 ;;
        *)            die "unknown option: $1 (try --help)" ;;
    esac
done
if [[ ${#explicit_steps[@]} -gt 0 ]]; then
    STEPS="${explicit_steps[*]}"
fi
has_step() { [[ " $STEPS " == *" $1 "* ]]; }

##### Step 0: Verify Build Environment
check_prerequisites() {
    step "Checking prerequisites"
    command -v brew >/dev/null || die "Homebrew is required: https://brew.sh"
    command -v git  >/dev/null || die "git is required (install the Xcode command line tools)"
    command -v cmake >/dev/null || die "cmake is required: brew install cmake"

    local missing=()
    for pkg in "${BREW_PACKAGES[@]}"; do
        brew --prefix "$pkg" >/dev/null 2>&1 || missing+=("$pkg")
    done
    if [[ ${#missing[@]} -gt 0 ]]; then
        die "missing Homebrew packages. Run: brew install ${missing[*]}"
    fi

    QT_PREFIX="$(brew --prefix qt)"
    ECM_PREFIX="$(brew --prefix extra-cmake-modules)"
    MACDEPLOYQT="$QT_PREFIX/bin/macdeployqt"
    [[ -x "$MACDEPLOYQT" ]] || die "macdeployqt not found at $MACDEPLOYQT"

    CMAKE_PREFIX_PATH_VALUE="$KF6_PREFIX;$QT_PREFIX;$ECM_PREFIX"

    log "Qt          $("$QT_PREFIX/bin/qmake" -query QT_VERSION) ($QT_PREFIX)"
    log "ECM         $ECM_PREFIX"
    log "KF6 target  $KF6_VERSION -> $KF6_PREFIX"
    log "Build type  $BUILD_TYPE, $JOBS jobs"
}

clean_tree() {
    step "Cleaning"
    rm -rf "$BUILD_DIR" "$DIST_DIR" "$KF6_PREFIX"
    # Keep the checkouts (they are expensive to re-clone) but drop their builds.
    for dir in "$DEPS_DIR"/*/build; do
        [[ -d "$dir" ]] && rm -rf "$dir"
    done
    return 0
}

##### Step 1: Build and Install KDE

fetch_package() {
    local group="$1" name="$2" tag="$3"
    local dir="$DEPS_DIR/$name"

    if [[ ! -d "$dir/.git" ]]; then
        log "Cloning $name $tag"
        git clone --depth 1 --branch "$tag" \
            "https://invent.kde.org/$group/$name.git" "$dir"
    fi

    # Reset to the pinned tag so a re-run is reproducible even if a previous
    # run patched the tree or left it dirty.
    if ! git -C "$dir" rev-parse -q --verify "refs/tags/$tag" >/dev/null; then
        log "Fetching $name $tag"
        git -C "$dir" fetch --depth 1 origin "refs/tags/$tag:refs/tags/$tag"
    fi
    git -C "$dir" checkout -q --detach "refs/tags/$tag"
    git -C "$dir" reset -q --hard "refs/tags/$tag"
    git -C "$dir" clean -qfd -e build

    for entry in "${KF6_PATCHES[@]}"; do
        local target="${entry%%:*}" patch="${entry#*:}"
        if [[ "$target" == "$name" ]]; then
            log "Applying $patch"
            git -C "$dir" apply "$SCRIPT_DIR/patches/$patch"
        fi
    done
}

build_package() {
    local name="$1"
    local dir="$DEPS_DIR/$name"

    step "Building $name"
    mkdir -p "$dir/build"
    cmake -S "$dir" -B "$dir/build" \
        -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH_VALUE" \
        -DCMAKE_INSTALL_PREFIX="$KF6_PREFIX" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOS_DEPLOYMENT_TARGET" \
        -DBUILD_TESTING=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DAPPLE_SUPPRESS_X11_WARNING=ON \
        > "$dir/build/configure.log" 2>&1 \
        || { cat "$dir/build/configure.log"; die "configure failed for $name"; }
    cmake --build "$dir/build" --parallel "$JOBS"
    cmake --install "$dir/build" >/dev/null
}

normalize_install_names() {
    # KDE occasionally emits an install name that does not match where the
    # library actually landed. Repair them so the bundling step can resolve
    # every dependency by path.
    step "Normalizing install names in $KF6_PREFIX"
    local fixed=0
    while IFS= read -r lib; do
        local current
        current="$(otool -D "$lib" | sed -n '2p')"
        if [[ "$current" != "$lib" ]]; then
            install_name_tool -id "$lib" "$lib"
            fixed=$((fixed + 1))
        fi
    done < <(find "$KF6_PREFIX/lib" -maxdepth 1 -name '*.dylib' -type f)
    log "repaired $fixed install name(s)"
}

build_deps() {
    mkdir -p "$DEPS_DIR"
    for entry in "${KF6_PACKAGES[@]}"; do
        local spec="${entry%@*}" tag="${entry##*@}"
        local group="${spec%%/*}" name="${spec##*/}"
        fetch_package "$group" "$name" "$tag"
        build_package "$name"
    done
    normalize_install_names
}

##### Step 2: Build the application

build_app() {
    step "Configuring ${APP_NAME}"
    cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
        -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH_VALUE" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOS_DEPLOYMENT_TARGET" \
        -DWITHOUT_QREADABLE=1 \
        -DAPPLE_SUPPRESS_X11_WARNING=ON \
        -DBUILD_TESTING=OFF

    step "Building ${APP_NAME}"
    cmake --build "$BUILD_DIR" --parallel "$JOBS"
    [[ -d "$BUILD_DIR/src/$APP_FILE_NAME.app" ]] || die "expected $BUILD_DIR/src/$APP_FILE_NAME.app"
}

##### Step 3: Make the bundle

copy_kf6_runtime() {
    step "Staging KDE Frameworks runtime"
    local contents="$APP_BUNDLE/Contents"
    mkdir -p "$contents/Frameworks" "$contents/PlugIns" "$contents/Resources/qml"

    # Shared libraries.
    find "$KF6_PREFIX/lib" -maxdepth 1 -name '*.dylib' -exec cp -a {} "$contents/Frameworks/" \;

    # QML modules
    if [[ -d "$KF6_PREFIX/lib/qml" ]]; then
        cp -R "$KF6_PREFIX/lib/qml/." "$contents/Resources/qml/"
    fi

    # Native plugins
    for dir in kf6 kiconthemes6 iconengines imageformats; do
        [[ -d "$KF6_PREFIX/lib/plugins/$dir" ]] && \
            cp -R "$KF6_PREFIX/lib/plugins/$dir" "$contents/PlugIns/"
    done

    # Translations
    if [[ -d "$KF6_PREFIX/share/locale" ]]; then
        mkdir -p "$contents/Resources/share"
        cp -R "$KF6_PREFIX/share/locale" "$contents/Resources/share/"
    fi

    # Strip build-time leftovers
    find "$contents/Resources/qml" "$contents/PlugIns" \
        \( -name '*.cmake' -o -name '*.prl' -o -name '*.a' -o -name '.DS_Store' \) \
        -delete 2>/dev/null || true
    return 0
}

prune_bundle() {
    step "Pruning unused Qt components"
    local contents="$APP_BUNDLE/Contents"
    for dir in "${PRUNE_PLUGIN_DIRS[@]}"; do
        rm -rf "$contents/PlugIns/$dir"
    done
    for file in "${PRUNE_PLUGIN_FILES[@]}"; do
        rm -f "$contents/PlugIns/$file"
    done
    for module in "${PRUNE_QML_MODULES[@]}"; do
        rm -rf "$contents/Resources/qml/$module"
    done
    # .qmltypes files are IDE metadata
    find "$contents/Resources/qml" -name '*.qmltypes' -delete 2>/dev/null || true
    return 0
}

write_qt_conf() {
    # Tell Qt to look inside the bundle for resources
    cat > "$APP_BUNDLE/Contents/Resources/qt.conf" <<'EOF'
[Paths]
Prefix = .
Plugins = ../PlugIns
Imports = qml
Qml2Imports = qml
Data = .
EOF
}

make_bundle() {
    step "Assembling $APP_BUNDLE"
    rm -rf "$APP_BUNDLE"
    mkdir -p "$DIST_DIR"
    cp -R "$BUILD_DIR/src/$APP_FILE_NAME.app" "$APP_BUNDLE"

    step "Running macdeployqt"
    "$MACDEPLOYQT" "$APP_BUNDLE" \
        -qmldir="$SOURCE_DIR/src" \
        -qmlimport="$KF6_PREFIX/lib/qml" \
        -verbose=1

    copy_kf6_runtime
    prune_bundle
    write_qt_conf

    step "Relocating dependencies"
    python3 "$SCRIPT_DIR/relocate.py" "$APP_BUNDLE" \
        --search "$KF6_PREFIX/lib" \
        --search "$QT_PREFIX/lib" \
        --gc

    step "Signing ($SIGN_IDENTITY)"
    codesign --force --sign "$SIGN_IDENTITY" --timestamp=none \
        $(find "$APP_BUNDLE/Contents/Frameworks" "$APP_BUNDLE/Contents/PlugIns" \
               "$APP_BUNDLE/Contents/Resources/qml" \
               -name '*.dylib' -o -name '*.so' -o -name '*.framework' 2>/dev/null) \
        2>/dev/null || true
    codesign --force --sign "$SIGN_IDENTITY" "$APP_BUNDLE"
    codesign --verify --deep --strict "$APP_BUNDLE" \
        || die "codesign verification failed"

    log "bundle size: $(du -sh "$APP_BUNDLE" | cut -f1)"
}

make_dmg() {
    step "Creating disk image"
    local dmg="$DIST_DIR/$APP_NAME.dmg"
    local staging="$DIST_DIR/.dmg-staging"
    rm -rf "$staging" "$dmg"
    mkdir -p "$staging"
    cp -R "$APP_BUNDLE" "$staging/"
    ln -s /Applications "$staging/Applications"
    hdiutil create -volname "$APP_NAME" -srcfolder "$staging" \
        -ov -format UDZO "$dmg" >/dev/null
    rm -rf "$staging"
    log "created $dmg"
}

##### Main
check_prerequisites
if [[ $CLEAN -eq 1 ]]; then clean_tree; fi

if has_step deps;   then build_deps;   fi
if has_step app;    then build_app;    fi
if has_step bundle; then make_bundle;  fi
if [[ $MAKE_DMG -eq 1 ]]; then make_dmg; fi

step "Done"
log "Bundle: $APP_BUNDLE"
