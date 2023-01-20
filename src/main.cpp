/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "application.h"
#include "cmake-config.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QQuickStyle>
using namespace FeedCore;

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    if (!qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_STYLE")) {
#ifdef ANDROID
        QQuickStyle::setStyle("assets:/MaterialTweaks");
        QQuickStyle::setFallbackStyle("Material");
#else
        QQuickStyle::setStyle("org.kde.desktop");
#endif
    }

    {
        QCommandLineParser commandLine;
        commandLine.addOption(QCommandLineOption("background"));
        commandLine.process(app);

        if (commandLine.isSet("background")) {
            app.startBackgroundNotifier();
        } else {
            app.loadMainWindow();
        }
    }

    int result = Application::exec();
    return result;
}
