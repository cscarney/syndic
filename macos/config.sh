# SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later
#

KF6_VERSION="v6.12.0"

# Frameworks in dependency order 
# <repo-group>/<name>@<tag>
KF6_PACKAGES=(
    "frameworks/kcodecs@${KF6_VERSION}"
    "frameworks/kconfig@${KF6_VERSION}"
    "frameworks/syndication@${KF6_VERSION}"
    "frameworks/breeze-icons@${KF6_VERSION}"
    "frameworks/kirigami@${KF6_VERSION}"
)

# Patches applied after checkout
# <package name>:<patch file name>
KF6_PATCHES=(
    "kirigami:kirigami-qt610-elapsedtimer.patch"
)

# Prerequisites that need to be installed with homebrew
BREW_PACKAGES=(qt extra-cmake-modules pkg-config)

# Qt-supplied stuff that we don't use
PRUNE_PLUGIN_DIRS=(sceneparsers renderers geometryloaders)
PRUNE_QML_MODULES=(QtQuick/Pdf Qt3D QtQuick3D)
PRUNE_PLUGIN_FILES=(imageformats/libqpdf.dylib quick/libpdfquickplugin.dylib)

# Name of the .app
APP_NAME="Syndic"
APP_FILE_NAME="syndic"

# Deployment target. Qt 6.10 from Homebrew requires macOS 12 or newer.
MACOS_DEPLOYMENT_TARGET="12.0"
