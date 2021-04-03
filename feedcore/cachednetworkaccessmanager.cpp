#include "cachednetworkaccessmanager.h"
#include <QStandardPaths>
#include <QNetworkDiskCache>
using namespace FeedCore;

CachedNetworkAccessManager::CachedNetworkAccessManager()
{
    auto *cache = new QNetworkDiskCache;
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/icons";
    cache->setCacheDirectory(cacheDir);
    setCache(cache);
}
