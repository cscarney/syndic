/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "networkaccessmanager.h"
#include <QStandardPaths>
#include <QNetworkDiskCache>
using namespace FeedCore;

FeedCore::NetworkAccessManager::NetworkAccessManager()
{
    auto *cache = new QNetworkDiskCache;
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    cache->setCacheDirectory(cacheDir);
    setCache(cache);
}
