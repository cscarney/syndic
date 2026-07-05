#!/bin/bash
# SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Build the Android APK locally in the builder container, the same way
# .github/workflows/android.yml does in CI, and copy the result out.
#
# Usage: scripts/build-android-apk.sh [output.apk]
#
# The output path defaults to ./syndic.apk. The build directory persists
# between runs, so rebuilds are incremental.
#
# Environment variables:
#   SYNDIC_ANDROID_IMAGE  builder image
#                         (default: ghcr.io/cscarney/syndic-builder-android)
#   SYNDIC_ANDROID_BUILD  host build directory (default: <repo>/build-android)
#   ANDROID_VERSION_CODE  android versionCode for the package (default: 1)
#   KEYSTORE              path to a keystore to sign with; requires
#   KEYSTORE_ALIAS, KEYSTORE_PASS and optionally KEY_PASS (defaults to
#                         KEYSTORE_PASS)
#
# Without KEYSTORE, a self-signed development key is generated on first use
# and kept in the build directory, so successive test builds share a stable
# signature and can be installed over each other.

set -euo pipefail

srcdir=$(cd "$(dirname "$0")/.." && pwd)
out=$(realpath -m "${1:-syndic.apk}")
image=${SYNDIC_ANDROID_IMAGE:-ghcr.io/cscarney/syndic-builder-android}
builddir=$(realpath -m "${SYNDIC_ANDROID_BUILD:-$srcdir/build-android}")

mkdir -p "$builddir"

docker_args=(
    --rm
    --user user
    -v "$srcdir":/work/src:ro
    -v "$builddir":/work/build
    -e ANDROID_VERSION_CODE="${ANDROID_VERSION_CODE:-1}"
)

if [ -n "${KEYSTORE:-}" ]; then
    : "${KEYSTORE_ALIAS:?KEYSTORE_ALIAS is required when KEYSTORE is set}"
    : "${KEYSTORE_PASS:?KEYSTORE_PASS is required when KEYSTORE is set}"
    docker_args+=(
        -v "$(realpath "$KEYSTORE")":/work/keystore.jks:ro
        -e QT_ANDROID_KEYSTORE_PATH=/work/keystore.jks
        -e QT_ANDROID_KEYSTORE_ALIAS="$KEYSTORE_ALIAS"
        -e QT_ANDROID_KEYSTORE_STORE_PASS="$KEYSTORE_PASS"
        -e QT_ANDROID_KEYSTORE_KEY_PASS="${KEY_PASS:-$KEYSTORE_PASS}"
    )
else
    echo "No KEYSTORE given; signing with the development key in $builddir" >&2
    docker_args+=(
        -e QT_ANDROID_KEYSTORE_PATH=/work/build/dev-keystore.jks
        -e QT_ANDROID_KEYSTORE_ALIAS=syndic-dev
        -e QT_ANDROID_KEYSTORE_STORE_PASS=android
        -e QT_ANDROID_KEYSTORE_KEY_PASS=android
        -e GENERATE_DEV_KEYSTORE=1
    )
fi

docker run "${docker_args[@]}" "$image" bash -ec '
    if [ -n "${GENERATE_DEV_KEYSTORE:-}" ] && [ ! -f "$QT_ANDROID_KEYSTORE_PATH" ]; then
        keytool -genkeypair -keystore "$QT_ANDROID_KEYSTORE_PATH" \
            -alias "$QT_ANDROID_KEYSTORE_ALIAS" -keyalg RSA -keysize 2048 \
            -validity 10000 -storepass "$QT_ANDROID_KEYSTORE_STORE_PASS" \
            -keypass "$QT_ANDROID_KEYSTORE_KEY_PASS" \
            -dname "CN=Syndic Development, O=Syndic, C=US"
    fi

    # craftenv.sh needs bash and changes the working directory; it also sets
    # CC to the NDK clang but leaves CXX unset, which breaks Craft host tools.
    source "$ANDROID_CRAFT_ROOT/craft/craftenv.sh" >/dev/null
    export CC=gcc CXX=g++

    cmake -S /work/src -B /work/build -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_QT_TOOLCHAIN" \
        -DCMAKE_PREFIX_PATH="$ANDROID_CRAFT_ROOT" \
        -DWITHOUT_QREADABLE=1 \
        -DANDROID_VERSION_CODE="$ANDROID_VERSION_CODE" \
        -DQT_ANDROID_SIGN_APK=ON

    # The signing key is read from the environment when androiddeployqt runs,
    # so an up-to-date APK from an earlier run would keep its old signature;
    # always repackage.
    rm -f /work/build/src/android-build/syndic.apk
    cmake --build /work/build --target apk
'

cp "$builddir/src/android-build/syndic.apk" "$out"
echo "APK written to $out"
