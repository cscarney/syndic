# Syndic
Syndic is a simple, responsive feed reader made for casual browsing. It adapts to both mouse and touch input, and runs on both desktop and mobile devices. The UI is designed with Plasma Desktop and Android in mind, but it should run in other environments as well.

![Screenshot](doc/screenshots/syndic-mid.png?raw=true)

## Getting Syndic

- You can get the [Linux desktop version](https://flathub.org/apps/details/com.rocksandpaper.syndic) from [Flathub](https://flathub.org/apps/details/com.rocksandpaper.syndic). This is the recommended way to install Syndic.
- Android apks for the current release are available from the [releases page](https://github.com/cscarney/syndic/releases).
- Pre-release flatpak bundles for Linux are also available from the [releases page](https://github.com/cscarney/syndic/releases).
- .deb packages for Ubuntu are available from the official [Launchpad PPA](https://launchpad.net/~cscarney/+archive/ubuntu/syndic). The PPA will only include Qt5 versions for now, since the Qt6 version has dependencies that aren't yet available in the Ubuntu repositories.
  
## Support and bug reporting
If you have a **question**, **comment** or **suggestion**, please open a thread on the [discussion board](https://github.com/cscarney/syndic/discussions).

If you have found a **bug in the application**, please check the [bug tracker](https://github.com/cscarney/syndic/issues) to see if it has already be reported, and open a new issue if necessary. 

**Please do not open bug reports to ask for help.** If you have a problem and you're not sure whether it's a bug or not, please start a discussion thread first. We can convert it to a bug report if necessary.

## Help Translate
Want to see Syndic in your native language? Contribute translations from your web browser using [weblate](https://hosted.weblate.org/projects/syndic/app/)!

Current translation status:

![Weblate translation status](https://hosted.weblate.org/widget/syndic/app/horizontal-auto.svg)

## Building from Source

The official git repository is [here](https://github.com/cscarney/syndic/).

The master branch builds against Qt6 and KDE Frameworks 6.  This is the branch you should build if you are interested in contributing to Syndic.

If you are only interested in *using* Syndic and want a stable, reproducible product, you should build one of the [release tags](https://github.com/cscarney/syndic/tags).

If you need to build against Qt5/KF5, you should use the [v1-series branch](https://github.com/cscarney/syndic/tree/v1-series).

It is recommended to build and install [QReadable](https://invent.kde.org/ccarney/qreadable) before building syndic. You can avoid the QReadable dependency for testing purposes by passing `-DWITHOUT_QREADABLE=1` to cmake, but this is not recommended, as it will result in a non-functional reader view.

Other required dependencies are Cmake, ECM, Qt6 (Core, Network, Qml, Quick, QQC, Sql), KF6 (Syndication, Config, Kirigami). KF6DbusAddons and Qt6Widgets are recommended on for desktop builds. Android builds additionally require OpenSSL.

Once all of the dependencies are installed you can build using the normal CMake commands:

    mkdir build && cd build
    cmake ..
    make
    make install

### Building for Android

Version 2.0 does not yet build for android. There are significant changes to android support in Qt6 that have not yet been implemented in Syndic. The [v1-series branch](https://github.com/cscarney/syndic/tree/v1-series) branch can be cross-compiled for android using Qt5.
