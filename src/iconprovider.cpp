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
    class IImpl
    {
    public:
        virtual ~IImpl() = default;
        virtual void respond(IconImageResponse *response) = 0;
        virtual void finish(IImpl * /* replacement */)
        {
        }
    };

    class PendingImpl : public IImpl
    {
        QList<IconImageResponse *> m_waitingResponses;
        void respond(IconImageResponse *response) override
        {
            m_waitingResponses << response;
        }

        void finish(IImpl *replacement) override
        {
            for (IconImageResponse *r : std::as_const(m_waitingResponses)) {
                replacement->respond(r);
            }
        }
    };

    class ImageImpl : public IImpl
    {
        QImage m_image;

        void respond(IconImageResponse *response) override
        {
            response->succeed(m_image);
        }

    public:
        explicit ImageImpl(const QImage &img)
            // store the image in the texture factory's preferred format
            // so that it doesn't detatch when we create the texture factory
            : m_image{img.convertToFormat(QImage::Format_ARGB32_Premultiplied)}
        {
        }
    };

    class RedirectImpl : public IImpl
    {
        QPointer<IconImageEntry> m_redirectTarget;
        void respond(IconImageResponse *response) override
        {
            if (m_redirectTarget.isNull()) {
                qWarning() << "Redirect target entry was destroyed.  This should never happen.";
                response->fail();
            }
            m_redirectTarget->respond(response);
        }

    public:
        explicit RedirectImpl(IconImageEntry *target)
            : m_redirectTarget{target}
        {
        }
    };

    class FailImpl : public IImpl
    {
        void respond(IconImageResponse *response) override
        {
            response->fail();
        }
    };

    std::unique_ptr<IImpl> d;

public:
    IconImageEntry(QNetworkAccessManager *nam, const QUrl &source, QObject *parent)
        : QObject(parent)
        , d{std::make_unique<PendingImpl>()}
    {
        QNetworkRequest req(source);
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
        QNetworkReply *reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply] {
            QImage img;
            QUrl redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            IImpl *newImpl{nullptr};

            if (redirectTarget.isValid()) {
                newImpl = new RedirectImpl(s_instance->getEntry(redirectTarget));
            } else if (reply->error() == QNetworkReply::NoError && img.loadFromData(reply->readAll())) {
                newImpl = new ImageImpl(img);
            } else {
                newImpl = new FailImpl;
            }
            d->finish(newImpl);
            d.reset(newImpl);
        });
    }

    void respond(IconImageResponse *response)
    {
        d->respond(response);
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
        getEntry(url)->respond(response);
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

IconProvider::IconImageEntry *IconProvider::getEntry(const QUrl &url)
{
    IconImageEntry *&pIcon = m_icons[url];
    if (pIcon == nullptr) {
        pIcon = new IconImageEntry(m_nam.get(), url, this);
    }
    return pIcon;
}
