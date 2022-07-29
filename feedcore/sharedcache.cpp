/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QMutex>
#include <QNetworkDiskCache>
#include <QStandardPaths>
#include <sharedcache.h>
using namespace FeedCore;

namespace
{
class SharedDiskCache : public QNetworkDiskCache
{
public:
    QMutex mutex;
    static SharedDiskCache *instance();

private:
    SharedDiskCache();
};

}

QNetworkCacheMetaData SharedCache::metaData(const QUrl &url)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->metaData(url);
}

void SharedCache::updateMetaData(const QNetworkCacheMetaData &metaData)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    SharedDiskCache::instance()->updateMetaData(metaData);
}

QIODevice *SharedCache::data(const QUrl &url)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->data(url);
}

bool SharedCache::remove(const QUrl &url)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->remove(url);
}

qint64 SharedCache::cacheSize() const
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->cacheSize();
}

QIODevice *SharedCache::prepare(const QNetworkCacheMetaData &metaData)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->prepare(metaData);
}

void SharedCache::insert(QIODevice *device)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    SharedDiskCache::instance()->insert(device);
}

void SharedCache::clear()
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    SharedDiskCache::instance()->clear();
}

SharedDiskCache *SharedDiskCache::instance()
{
    static auto *instance = new SharedDiskCache;
    return instance;
}

SharedDiskCache::SharedDiskCache()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    setCacheDirectory(cacheDir);
}
