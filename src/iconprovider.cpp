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
class IconCache : public QNetworkDiskCache
{
public:
    IconCache();
    QIODevice *prepare(const QNetworkCacheMetaData &metaData) override;
    static constexpr const int kReasonableMaxAge = 86400;

    // enough space for any concievable collection of icons; 1GB should be enough
    static constexpr const qint64 kVeryLargeCacheSize = 1U << 30U;
};

IconCache::IconCache()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/icons/");
    setCacheDirectory(cacheDir);
    setMaximumCacheSize(kVeryLargeCacheSize);
}

QIODevice *IconCache::prepare(const QNetworkCacheMetaData &metaData)
{
    QNetworkCacheMetaData newMetaData(metaData);

    // make sure that the server's cache expiration isn't unreasonably short
    QDateTime reasonableExpirationDate = QDateTime::currentDateTime().addSecs(kReasonableMaxAge);
    if (metaData.expirationDate() < reasonableExpirationDate) {
        newMetaData.setExpirationDate(reasonableExpirationDate);
    }

    // store even if the server says not to
    newMetaData.setSaveToDisk(true);

    return QNetworkDiskCache::prepare(newMetaData);
}

}

// NB: this class belongs to the requesting thread
class IconProvider::IconImageResponse : public QQuickImageResponse
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

    void succeed(const QImage &image)
    {
        QMetaObject::invokeMethod(this, [this, image] {
            m_image = image;
            emit finished();
        });
    }

    void fail()
    {
        QMetaObject::invokeMethod(this, [this] {
            m_error = "failed to load image";
            emit finished();
        });
    }
};

class IconProvider::IconImageEntry : public QObject
{
    enum Status { Pending, Success, Error };

    Status m_status{Pending};
    QImage m_image;
    QList<IconImageResponse *> m_waitingResponses;

public:
    IconImageEntry(QNetworkAccessManager *nam, const QUrl &source, QObject *parent)
        : QObject(parent)
    {
        QNetworkRequest req(source);
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply *reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply] {
            if (reply->error() == QNetworkReply::NoError && m_image.loadFromData(reply->readAll())) {
                m_status = Success;
            } else {
                m_status = Error;
            }
            finishWaitingResponses();
        });
    }

    void respond(IconImageResponse *response)
    {
        if (m_status == Pending) {
            m_waitingResponses.append(response);
        } else {
            finishResponse(response);
        }
    }

private:
    void finishWaitingResponses()
    {
        for (auto *response : qAsConst(m_waitingResponses)) {
            finishResponse(response);
        }
        m_waitingResponses.clear();
    }

    void finishResponse(IconImageResponse *response)
    {
        if (m_status == Success) {
            response->succeed(m_image);
        } else {
            response->fail();
        }
    }
};

IconProvider *IconProvider::s_instance{nullptr};

IconProvider::IconProvider()
    : m_nam{new FeedCore::NetworkAccessManager(new IconCache)}
{
    m_nam->setAutoDeleteReplies(true);
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
    QTimer::singleShot(0, this, [this, id, response] {
        QUrl url{id};
        IconImageEntry *&pIcon = m_icons[url];
        if (pIcon == nullptr) {
            pIcon = new IconImageEntry(m_nam.get(), url, this);
        }
        pIcon->respond(response);
    });
    return response;
}

// TODO this should be refactored into it's own class, probably in core
void IconProvider::discoverIcon(FeedCore::Feed *feed)
{
    if (!feed->icon().isEmpty()) {
        return;
    }

    if (s_instance == nullptr) {
        qWarning() << "discoverIcon called before creating an IconProvider";
    }

    // if we don't have a feed link, wait and see if we get one in the next update
    if (feed->link().isEmpty()) {
        auto *context = new QObject(feed);
        QObject::connect(feed, &FeedCore::Feed::statusChanged, context, [feed, context] {
            if (feed->status() == FeedCore::Feed::Idle) {
                if (!feed->link().isEmpty()) {
                    discoverIcon(feed);
                }
                QObject::disconnect(feed, nullptr, context, nullptr);
            }
        });
        return;
    }

    QUrl iconUrl(feed->link());
    iconUrl.setPath("/favicon.ico");
    QNetworkRequest req(iconUrl);
    QNetworkReply *reply = s_instance->m_nam->get(req);
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
}
