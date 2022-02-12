/*
    SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ANDROIDSTYLEPLUGINFACTORY_H
#define ANDROIDSTYLEPLUGINFACTORY_H

#include <Kirigami2/KirigamiPluginFactory>

class AndroidStylePluginFactory : public Kirigami::KirigamiPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kirigami.KirigamiPluginFactory" FILE "androidstyleplugin.json")
    Q_INTERFACES(Kirigami::KirigamiPluginFactory)

public:
    explicit AndroidStylePluginFactory(QObject *parent = nullptr);
    Kirigami::PlatformTheme *createPlatformTheme(QObject *parent) override;
};

#endif
