# Syndic
Syndic is a simple, responsive feed reader made for casual browsing. It adapts to both mouse and touch input, and runs on both desktop and mobile devices. The UI is designed with Plasma Desktop and Android in mind, but it should run in other environments as well.

![Screenshot](doc/screenshots/syndic-mid.png?raw=true)

## Getting Syndic

- Flatpak builds are available on [Flathub](https://flathub.org/apps/details/com.rocksandpaper.syndic)
- Android apks for the current release are available from the [releases page](https://github.com/cscarney/syndic/releases).
- .deb packages for Ubuntu are available from our [Launchpad PPA](https://launchpad.net/~cscarney/+archive/ubuntu/syndic).

## Building from Source

This branch currently builds against the development version of KF6 with Qt6. If you have Plasma 5/KF5, you should be building from the [v1-series branch](https://github.com/cscarney/syndic/tree/v1-series) instead of this one.

You will need to build and install [QReadable](https://invent.kde.org/ccarney/qreadable) before building syndic. Other required dependencies are Cmake, ECM, Qt6 (Core, Network, Qml, Quick, QQC2, Sql), KF6 (Syndication, Config, Kirigami). KF6DbusAddons and Qt Widgets are recommended on for desktop builds. Android builds additionally require OpenSSL and Qt AndroidExtras.

The KF6 dependencies are currently in development and you will probably need to build them yourself with `kdesrc-build`.

Once all of the dependencies are installed you can build using the normal CMake commands:

    mkdir build && cd build
    cmake ..
    make
    make install

### Building for Android

[Cross Compiling for Android](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android) is beyond the scope of this readme, but in general, you should use the [ECM Android Toolchain](https://api.kde.org/ecm/toolchain/Android.html) with `-DQTANDROID_EXPORTED_TARGET=syndic -DANDROID_APK_DIR=/path/to/source/android` and then make the `create-apk-syndic` target. 

## Help Translate
Want to see Syndic in your native language? Contribute translations from your web browser using [weblate](https://hosted.weblate.org/projects/syndic/app/)!
