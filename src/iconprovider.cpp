/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "iconprovider.h"
#include "feed.h"
#include "networkaccessmanager.h"
#include <QBuffer>
#include <QImageReader>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTimer>

namespace
{
class IconImageResponse : public QQuickImageResponse
{
public:
    QImage m_image;
    QString m_error;

    QString errorString() const override
    {
        return m_error;
    }

    QQuickTextureFactory *textureFactory() const override
    {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    void onNetworkReplyFinished()
    {
        auto *reply = qobject_cast<QNetworkReply *>(QObject::sender());
        bool success = m_image.loadFromData(reply->readAll());
        if (!success) {
            m_error = "failed to load image";
        }
        reply->deleteLater();
        emit finished();
    }
};

class IconCache : public QNetworkDiskCache
{
public:
    IconCache();
    QIODevice *prepare(const QNetworkCacheMetaData &metaData) override;
    static constexpr const int kReasonableMaxAge = 86400;
};

IconCache::IconCache()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/icons/");
    setCacheDirectory(cacheDir);
}

QIODevice *IconCache::prepare(const QNetworkCacheMetaData &metaData)
{
    // make sure that the server's cache expiration isn't unreasonably short
    QDateTime reasonableExpirationDate = QDateTime::currentDateTime().addSecs(kReasonableMaxAge);
    if (metaData.expirationDate() < reasonableExpirationDate) {
        QNetworkCacheMetaData newMetaData(metaData);
        newMetaData.setExpirationDate(reasonableExpirationDate);
        return QNetworkDiskCache::prepare(newMetaData);
    }
    return QNetworkDiskCache::prepare(metaData);
}

}

IconProvider *IconProvider::s_instance{nullptr};

IconProvider::IconProvider()
    : m_nam{new FeedCore::NetworkAccessManager(new IconCache)}
{
    s_instance = this;
}

IconProvider::~IconProvider()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

QQuickImageResponse *IconProvider::requestImageResponse(const QString &id, const QSize & /*requestedSize */)
{
    auto *response = new IconImageResponse;
    auto *nam = m_nam.get();
    QTimer::singleShot(0, nam, [nam, id, response] {
        QNetworkRequest req(id.mid(1));
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply *reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, response, &IconImageResponse::onNetworkReplyFinished);
    });
    return response;
}

void IconProvider::discoverIcon(FeedCore::Feed *feed)
{
    if (feed->link().isEmpty() || s_instance == nullptr) {
        return;
    }
    QTimer::singleShot(0, feed, [feed, nam = s_instance->m_nam] {
        QUrl iconUrl(feed->link());
        iconUrl.setPath("/favicon.ico");
        QNetworkRequest req(iconUrl);
        QNetworkReply *reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, feed, [reply, feed] {
            if (reply->error() != QNetworkReply::NoError) {
                // couldn't get file
                return;
            }
            QByteArray data = reply->readAll();
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QImageReader reader(&buffer);
            bool success = reader.canRead();
            if (success) {
                feed->setIcon(reply->url());
            } else {
                // unreadable image
            }
        });
    });
}
