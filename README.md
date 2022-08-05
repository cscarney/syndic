# Syndic
Syndic is a simple, responsive feed reader designed for casual browsing. It is designed to adapt to either mouse and touch input, and run well on both desktop and mobile devices. The UI is designed with Plasma Desktop and Android in mind, but it should run in other environments as well.

![Screenshot](doc/screenshots/syndic-mid.png?raw=true)

## Getting Syndic

Flatpaks and apks for the current release are available from the [releases page](https://github.com/cscarney/syndic/releases).
.deb packages for Ubuntu are available from our [Launchpad PPA](https://launchpad.net/~cscarney/+archive/ubuntu/syndic).

## Building from Source

If you are looking to build a current, stable version of, you should build from the [v1-series branch](https://github.com/cscarney/syndic/tree/v1-series), or from a [release tag](https://github.com/cscarney/syndic/tags).

Required dependencies are Cmake, ECM, Qt5 (Core, Network, Qml, Quick, QQC2, Sql), KF5 (Syndication, Config, Kirigami). KF5DbusAddons and Qt Widgets are recommended on for desktop builds. Android builds additionally require OpenSSL and Qt AndroidExtras.

On Ubuntu/Debian, you can install all of the required dependencies with:

    apt install kirigami-dev cmake extra-cmake-modules libkf5config-dev libkf5dbusaddons-dev libkf5syndication-dev qtbase5-dev qtdeclarative5-dev qtquickcontrols2-5-dev qttools5-dev

Once all of the dependencies are installed you can build using the normal CMake commands:

    mkdir build && cd build
    cmake ..
    make
    make install

### Building for Android

[Cross Compiling for Android](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android) is beyond the scope of this readme, but in general, you should use the [ECM Android Toolchain](https://api.kde.org/ecm/toolchain/Android.html) with `-DQTANDROID_EXPORTED_TARGET=syndic -DANDROID_APK_DIR=/path/to/source/android` and then make the `create-apk-syndic` target. 

You MUST build Qt with the [KDE Patch Collection](https://community.kde.org/Qt5PatchCollection) when targeting android, as there are critical bugs in the current (5.15.4 as of this writing) open source release that WILL cause crashes.

## Help Translate
Want to see Syndic in your native language? Contribute translations from your web browser using [weblate](https://hosted.weblate.org/projects/syndic/app/)!
