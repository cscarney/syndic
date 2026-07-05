<!--
SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
SPDX-License-Identifier: GPL-3.0-or-later
-->

# Android build system

Syndic builds for Android (arm64-v8a) as a Qt6/KF6 application. Everything
runs inside a purpose-built container image, both in CI and locally, so no
Android SDK/NDK or cross-compiled Qt needs to be installed on the host.

Components:

| Piece | Location | Purpose |
|---|---|---|
| Builder image | `.github/android-builder/Dockerfile` | Toolchain + Qt6-for-Android + KF6 deps |
| Image publish workflow | `.github/workflows/android-builder.yml` | Builds/pushes the image to GHCR |
| APK build workflow | `.github/workflows/android.yml` | Builds (and signs) the APK in CI |
| Local build script | `scripts/build-android-apk.sh` | Same build on a developer machine |
| CMake integration | `CMakeLists.txt`, `src/CMakeLists.txt` | Android-specific packaging logic |

## The builder image

Published as `ghcr.io/cscarney/syndic-builder-android`. It derives from KDE's
Android CI image (`invent-registry.kde.org/sysadmin/ci-images/android-qt611-ci`),
which provides:

- Android SDK (platform 36, build-tools 36) and NDK, JDK 17, CMake, Ninja
- Qt 6.11 prebuilt for Android in a Craft root at
  `/home/user/android-arm64-clang` (toolchain file at
  `$ANDROID_QT_TOOLCHAIN`, i.e. `<root>/lib/cmake/Qt6/qt.toolchain.cmake`)
- Matching host-side Qt tooling in `/opt/nativetooling` (`QT_HOST_PATH`) and
  KF6 host tools in `/opt/nativetooling6`

The KDE image is a *stripped Craft artifact*: the Craft tool itself and the
KDE blueprints were deleted after KDE's deploy. Our Dockerfile restores both
(`git clone` of craft and craft-blueprints-kde) and then runs
`craft kconfig syndication kirigami`, which downloads Syndic's KF6
dependencies from KDE's binary cache into the Craft root — no Qt or KF6 is
compiled during the image build.

Quirks the Dockerfile works around (see its comments):

- **Host tool overlay.** KF6 build-time code generators
  (`kconfig_compiler_kf6`, `kconf_update`) installed into the Craft root are
  arm64 binaries; the KF6 CMake config imports them from the target prefix,
  so they must run on the x86_64 build host. The Dockerfile overlays the
  x86_64 copies from `/opt/nativetooling6`.
- **GitLab-isms.** The KDE image ends with `USER user` and a `/home/user`
  workdir, which break GitHub Actions job containers (the runner bind-mounts
  workspaces owned by other uids and expects root). The Dockerfile resets to
  `USER root` / `WORKDIR /`.
- **Node.** JavaScript-based actions (checkout, upload-artifact) need node.
  Real GitHub runners mount their own copy into job containers, but
  [act](https://github.com/nektos/act) uses the image's, so node 20 is baked
  in for local workflow testing.

The image rebuilds automatically when the Dockerfile changes on master, or on
manual dispatch of the *Android Builder Image* workflow. Note that a rebuild
picks up whatever KDE's binary cache currently has — the KF6 versions are not
pinned.

## Building inside the container

The canonical command sequence (what both the workflow and the script run):

```bash
source "$ANDROID_CRAFT_ROOT/craft/craftenv.sh"   # bash only!
export CC=gcc CXX=g++
cmake -S <src> -B <build> -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_QT_TOOLCHAIN \
    -DCMAKE_PREFIX_PATH=$ANDROID_CRAFT_ROOT \
    -DWITHOUT_QREADABLE=1
cmake --build <build> --target apk
```

The APK lands at `<build>/src/android-build/syndic.apk` (release, unsigned
unless signing is configured — see below).

Gotchas encoded in that sequence:

- `craftenv.sh` is a bash script (it defines shell functions) **and it
  changes the working directory** to the Craft root — always pass absolute
  `-S`/`-B` paths after sourcing it.
- `craftenv.sh` sets `CC` to the NDK clang but leaves `CXX` unset, which
  breaks host tooling that some builds invoke; resetting both to the host
  compilers is safe because the CMake toolchain file selects the NDK
  compilers for target code explicitly.
- QReadable (the reader view) has no Android port yet, hence
  `-DWITHOUT_QREADABLE=1`.

## CMake integration

All Android-specific logic is guarded by `if (ANDROID)`; desktop builds are
unaffected. The non-obvious parts, in the order they bit us:

- **`qt_add_executable` instead of `add_executable`** (`src/CMakeLists.txt`).
  On Android, Qt builds the "executable" as a shared MODULE library and the
  target finalizer generates the androiddeployqt deployment settings and the
  `apk`/`aab` targets. This also requires all `target_link_libraries` calls
  on the target to use keyword (`PRIVATE`) form, because Qt's finalizer uses
  it.
- **`PREFIX "lib"`** (`src/CMakeLists.txt`). androiddeployqt expects the app
  module to be named `lib<target>_<abi>.so`, but KDECMakeSettings globally
  clears `CMAKE_SHARED_MODULE_PREFIX` (KDE plugin convention), producing
  `syndic_arm64-v8a.so` and a "Cannot find application binary" packaging
  failure. The target property restores the prefix.
- **Package source dir is assembled in the build tree** (top-level
  `CMakeLists.txt`, `android-package-source` target).
  `QT_ANDROID_PACKAGE_SOURCE_DIR` points at `<build>/android-package`, which
  is populated from the static files in `android/` (manifest, res, java
  sources, assets) plus generated files. Do **not** try to stage generated
  files into `<build>/src/android-build/` directly — androiddeployqt wipes
  and regenerates that directory on every run.
- **Translations.** The qm loader generated by `ecm_create_qm_loader()` has a
  built-in Android branch that loads catalogs from
  `assets:/share/locale/<lang>/LC_MESSAGES/syndic.qm`. The install step that
  would normally lay out `share/locale` never runs for APK builds, so the
  catalogs compiled by `ecm_install_po_files_as_qm(po)` (which land in
  `<build>/ECMPoQm/<lang>/`) are copied into the package source dir's
  `assets/` and travel into the APK from there.
- **KDE QML modules** (`src/CMakeLists.txt`). androiddeployqt's
  qmlimportscanner only scans Qt's own `qml/` directory, but KDE frameworks
  install QML modules under `lib/qml` in the prefix. Without appending each
  prefix's `lib/qml` to the target's `QT_QML_IMPORT_PATH`, the Kirigami QML
  plugins are silently omitted and the app aborts at launch with
  `module "org.kde.kirigami" plugin ... not found`. (The Kirigami *shared
  libraries* still get bundled either way — they're link dependencies — which
  makes the omission easy to miss.)
- **Ordering.** Custom staging targets are hooked into `syndic_make_apk` /
  `syndic_make_aab` via `add_dependencies`. Hanging them off the `syndic`
  target does not work: when the library is up to date, ninja never orders
  the staging before androiddeployqt.
- **Versioning.** `QT_ANDROID_VERSION_NAME` comes from the project version;
  `versionCode` comes from the `ANDROID_VERSION_CODE` cache variable
  (default 1). CI passes `github.run_number`, which keeps it monotonic.

## Signing

Qt's native mechanism is used: configuring with `-DQT_ANDROID_SIGN_APK=ON`
makes the `apk` target invoke androiddeployqt with `--sign`, which reads the
keystore from the environment **at packaging time**:

```
QT_ANDROID_KEYSTORE_PATH        path to the keystore file
QT_ANDROID_KEYSTORE_ALIAS       key alias
QT_ANDROID_KEYSTORE_STORE_PASS  keystore password
QT_ANDROID_KEYSTORE_KEY_PASS    key password (defaults to the store password)
```

Because the key material is read from the environment, ninja does not know
about it: an already-up-to-date APK keeps its previous signature. Delete
`<build>/src/android-build/syndic.apk` to force a repackage when only the
signing configuration changed (the local script always does this).

### CI secrets

The *Android APK* workflow signs when these repository secrets exist:

| Secret | Content |
|---|---|
| `ANDROID_KEYSTORE` | keystore file, base64-encoded (`base64 -w0 keystore.jks`) |
| `ANDROID_KEYSTORE_ALIAS` | key alias |
| `ANDROID_KEYSTORE_PASSWORD` | keystore password |
| `ANDROID_KEY_PASSWORD` | key password |

Builds without the secrets (e.g. pull requests from forks) produce an
unsigned APK, which cannot be installed but still validates the build.

## CI workflows

- **`android-builder.yml`** – builds the Dockerfile and pushes
  `ghcr.io/<owner>/syndic-builder-android:latest`. The image is ~14 GB
  unpacked, so the job first clears preinstalled toolchains off the runner
  disk.
- **`android.yml`** – runs on pushes to master, version tags, PRs, and manual
  dispatch. The job executes inside the builder image (job `container:`),
  imports the keystore secret if present, configures, builds the `apk`
  target, and uploads `syndic-android-arm64` as a build artifact. Job-level
  `defaults.run.shell: bash` is required for `source craftenv.sh` (and for
  act, which otherwise uses `sh`).

Both workflows can be exercised locally with `gh act`; pass
`--pull=false` (to use the local image), a `--secret-file` with test
secrets, and `--artifact-server-path` for the upload step.

## Local builds

```bash
scripts/build-android-apk.sh [output.apk]
```

Wraps the container build with a persistent build directory
(`<repo>/build-android` by default, override with `SYNDIC_ANDROID_BUILD`) for
incremental rebuilds, and copies the APK to the given path. Signing uses
`KEYSTORE`/`KEYSTORE_ALIAS`/`KEYSTORE_PASS`/`KEY_PASS` if set; otherwise a
self-signed development key is generated once into the build directory and
reused, so successive test builds keep a stable signature and can be
installed over each other. Remember that Android ties app identity to the
signing certificate: switching between differently-signed builds requires an
uninstall.

## Current limitations

- Single ABI (arm64-v8a).
- No reader view: QReadable is not ported to Android.
- `androidstyleplugin/` (Kirigami Material style tweaks) is still Qt5-era
  and excluded from the build; Android uses stock Kirigami styling.
- KF6 dependency versions in the builder image float with KDE's binary cache
  rather than being pinned.
