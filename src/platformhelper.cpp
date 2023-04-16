/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "platformhelper.h"
#include "cmake-config.h"
#ifdef ANDROID

#include <QAndroidJniObject>

PlatformHelper::PlatformHelper(QObject *parent)
    : QObject(parent)
{
}

void PlatformHelper::share(const QUrl &url)
{
    QAndroidJniObject javaUrlString = QAndroidJniObject::fromString(url.toString());
    QAndroidJniObject::callStaticMethod<void>("com/rocksandpaper/syndic/NativeHelper", "sendUrl", "(Ljava/lang/String;)V", javaUrlString.object<jstring>());
}

void PlatformHelper::configureBackgroundService(bool)
{
    // background running not currently supported on android
}

#else
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDebug>
#include <QDesktopServices>
#include <QVariant>

PlatformHelper::PlatformHelper(QObject *parent)
    : QObject(parent)
{
}

void PlatformHelper::share(const QUrl &url)
{
    QString encodedUrl = QUrl::toPercentEncoding(url.toString());
    QDesktopServices::openUrl(QLatin1String("mailto:?body=") + encodedUrl);
}

void PlatformHelper::configureBackgroundService(bool enabled)
{
#ifdef KF6DBusAddons_FOUND
    QDBusInterface backgroundPortal("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Background");
    if (!backgroundPortal.isValid()) {
        qDebug() << "Not configuring autostart because background portal wasn't available";
        return;
    }
    QString executableName = qApp->applicationFilePath();
    QVariantMap options{{"autostart", enabled}, {"commandline", QVariant::fromValue(QStringList{executableName, "--background"})}};
    backgroundPortal.call("RequestBackground", "", options);
#endif
}

#endif
