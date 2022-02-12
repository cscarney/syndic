/*
    SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "androidstylepluginfactory.h"
#include "androidstyleplugintheme.h"

AndroidStylePluginFactory::AndroidStylePluginFactory(QObject *parent)
    : Kirigami::KirigamiPluginFactory(parent)
{
}

Kirigami::PlatformTheme *AndroidStylePluginFactory::createPlatformTheme(QObject *parent)
{
    return new AndroidStylePluginTheme(parent);
}
