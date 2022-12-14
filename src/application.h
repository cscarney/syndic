/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "cmake-config.h"
#include <QQmlApplicationEngine>
#include <memory>

#ifdef Qt5Widgets_FOUND
#include <QApplication>
typedef QApplication SyndicApplicationBase;
#else
#include <QGuiApplication>
typedef QGuiApplication SyndicApplicationBase;
#endif

namespace FeedCore
{
class Context;
}
class Settings;
class NotificationController;

class Application : public SyndicApplicationBase
{
public:
    Application(int &argc, char **argv);
    ~Application();
    FeedCore::Context *context();
    Settings *settings();
    void loadMainWindow();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    void activateMainWindow();
    void unloadEngine();
    void onLastWindowClosed();
    void onActivateRequested(const QStringList &arguments, const QString &workingDirectory);
    void bindContextPropertiesToSettings();
    void syncAutomaticUpdates();
    void syncDefaultUpdateInterval();
    void syncExpireAge();
    void startNotifications();
};
