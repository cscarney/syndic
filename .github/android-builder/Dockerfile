# SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Builder image for Syndic Android CI builds.
#
# Based on KDE's Android CI image, which ships a prebuilt Qt6-for-Android in a
# Craft root (/home/user/android-arm64-clang) plus the matching host-side Qt
# tooling (/opt/nativetooling, /opt/nativetooling6). The KDE image is designed
# for GitLab CI and ends with a non-root USER and a /home/user WORKDIR that
# break GitHub Actions job containers; the directives at the bottom of this
# file reset both, so the published image is GitHub-runner-friendly.
FROM invent-registry.kde.org/sysadmin/ci-images/android-qt611-ci:latest

USER user

# The CI image is a stripped Craft artifact: the craft tool itself and the KDE
# blueprints were removed after the deploy. Restore them, then pull Syndic's
# KF6 dependencies from KDE's binary cache (UseCache is preconfigured in the
# Craft root, so this reuses the prebuilt Qt6 instead of rebuilding it).
#
# craftenv.sh defines `craft` as a bash function, so this must run under bash.
# craftenv.sh sets CC to the NDK clang but leaves CXX unset, which crashes
# Craft's PipBuildSystem when it builds host python modules; point both at the
# host compilers (Craft passes the NDK toolchain to target builds explicitly).
RUN set -eux; \
    git clone --depth=1 https://invent.kde.org/packaging/craft.git /home/user/craft-clone; \
    git clone --depth=1 https://invent.kde.org/packaging/craft-blueprints-kde.git /home/user/craftmaster/blueprints; \
    bash -ec 'source /home/user/android-arm64-clang/craft/craftenv.sh; \
              export CC=gcc CXX=g++; \
              craft --version; \
              craft kconfig syndication kirigami'

# KF6 build-time code generators (kconfig_compiler_kf6 etc.) installed into
# the Craft root are arm64 binaries that can't run on the x86_64 build host,
# but the KF6 cmake config files import them from the target prefix. Overlay
# the host copies that the base image provides in /opt/nativetooling6.
RUN set -eux; \
    hsrc=/opt/nativetooling6/lib/x86_64-linux-gnu/libexec/kf6; \
    hdst=/home/user/android-arm64-clang/lib/libexec/kf6; \
    for t in "$hsrc"/*; do cp -f "$t" "$hdst/$(basename "$t")"; done

ENV ANDROID_CRAFT_ROOT=/home/user/android-arm64-clang
ENV ANDROID_QT_TOOLCHAIN=/home/user/android-arm64-clang/lib/cmake/Qt6/qt.toolchain.cmake
ENV CMAKE_PREFIX_PATH=/home/user/android-arm64-clang

# GitHub Actions job containers must run as root: the runner bind-mounts the
# workspace and tool cache with uids that don't match the image's `user`, and
# it overrides the working directory to /__w/... at container start.
USER root
WORKDIR /

# Node for javascript-based actions (checkout, upload-artifact). Real GitHub
# runners mount their own node into job containers, but act uses the image's,
# and the base image has none.
RUN set -eux; \
    curl -fsSL https://nodejs.org/dist/v20.20.2/node-v20.20.2-linux-x64.tar.xz \
    | tar -xJ -C /usr/local --strip-components=1
