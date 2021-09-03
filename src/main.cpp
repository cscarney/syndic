/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "cmake-config.h"
#include <QQuickStyle>
#include "application.h"
using namespace FeedCore;


int main(int argc, char *argv[])
{
    Application app(argc, argv);

#ifdef ANDROID
    QQuickStyle::setStyle("Material");
#else
    QQuickStyle::setStyle("org.kde.desktop");
#endif

    app.loadMainWindow();

    int result = Application::exec();
    return result;
}
