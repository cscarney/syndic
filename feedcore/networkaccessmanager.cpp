/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "networkaccessmanager.h"
#include <QDebug>
#include <QMutex>
#include <QNetworkDiskCache>
#include <QStandardPaths>
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

/* This shim class allows multiple instances of our NAM to use the same disk cache.
 *
 * This is necessary so that we can safely return instances of our NAM from QNetworkAccessManagerFactory.
 */
class SharedCacheProxy : public QAbstractNetworkCache
{
public:
    SharedCacheProxy() = default;
    QNetworkCacheMetaData metaData(const QUrl &url) override;
    void updateMetaData(const QNetworkCacheMetaData &metaData) override;
    QIODevice *data(const QUrl &url) override;
    bool remove(const QUrl &url) override;
    qint64 cacheSize() const override;
    QIODevice *prepare(const QNetworkCacheMetaData &metaData) override;
    void insert(QIODevice *device) override;
    void clear() override;

private:
    static SharedDiskCache *getShared();
};

}

QNetworkCacheMetaData SharedCacheProxy::metaData(const QUrl &url)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->metaData(url);
}

void SharedCacheProxy::updateMetaData(const QNetworkCacheMetaData &metaData)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    SharedDiskCache::instance()->updateMetaData(metaData);
}

QIODevice *SharedCacheProxy::data(const QUrl &url)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->data(url);
}

bool SharedCacheProxy::remove(const QUrl &url)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->remove(url);
}

qint64 SharedCacheProxy::cacheSize() const
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->cacheSize();
}

QIODevice *SharedCacheProxy::prepare(const QNetworkCacheMetaData &metaData)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    return SharedDiskCache::instance()->prepare(metaData);
}

void SharedCacheProxy::insert(QIODevice *device)
{
    QMutexLocker lock(&SharedDiskCache::instance()->mutex);
    SharedDiskCache::instance()->insert(device);
}

void SharedCacheProxy::clear()
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

NetworkAccessManager *NetworkAccessManager::instance()
{
    static auto *singleton = new NetworkAccessManager();
    return singleton;
}

FeedCore::NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : NetworkAccessManager(new SharedCacheProxy, parent)
{
}

NetworkAccessManager::NetworkAccessManager(QAbstractNetworkCache *cache, QObject *parent)
    : QNetworkAccessManager(parent)
{
    setCache(cache);
}

QNetworkReply *FeedCore::NetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QNetworkRequest newRequest(request);
    newRequest.setHeader(QNetworkRequest::UserAgentHeader, "syndic/1.0");
    newRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    newRequest.setTransferTimeout();
    return QNetworkAccessManager::createRequest(op, newRequest, outgoingData);
}
