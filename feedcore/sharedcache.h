/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FEEDCORE_SHAREDCACHE_H
#define FEEDCORE_SHAREDCACHE_H

#include <QAbstractNetworkCache>

namespace FeedCore
{

/**
 * A cache implementation that shares cached data across all of its instances
 *
 * The SharedCache class is a thin wrapper which provides thread-safe
 * access to a single global QNetworkDiskCache instance.
 */
class SharedCache : public QAbstractNetworkCache
{
public:
    SharedCache() = default;

    QNetworkCacheMetaData metaData(const QUrl &url) override;
    void updateMetaData(const QNetworkCacheMetaData &metaData) override;
    QIODevice *data(const QUrl &url) override;
    bool remove(const QUrl &url) override;
    qint64 cacheSize() const override;
    QIODevice *prepare(const QNetworkCacheMetaData &metaData) override;
    void insert(QIODevice *device) override;
    void clear() override;
};

}

#endif // SHAREDCACHE_H
