/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "updatablefeed.h"
#include "feeddiscovery.h"
#include "networkaccessmanager.h"
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>
#include <Syndication/Image>
#include <Syndication/ParserCollection>
using namespace FeedCore;

constexpr const int kMaxRedirects = 10;

namespace
{
class LoadOperation : public QObject
{
    Q_OBJECT
public:
    void start(const QUrl &url, const QString &failMessage = QString());
    void abort();

    struct DeleteLater {
        explicit DeleteLater(LoadOperation *t)
        {
            t->deleteLater();
        }
    };

signals:
    void succeeded(const Syndication::FeedPtr &feed, const QUrl &changeUrl, DeleteLater); // NOLINT
    void failed(const QString &errorString, DeleteLater); // NOLINT
    void aborted(DeleteLater); // NOLINT

private:
    QSet<QUrl> m_seenUrls;
    QPointer<QNetworkReply> m_reply;
    bool m_isDiscoveredFeed{false};
    void onReplyFinished();
};
}

class UpdatableFeed::UpdaterImpl : public Feed::Updater
{
public:
    UpdaterImpl(UpdatableFeed *feed, QObject *parent);
    void run() final;
    void abort() final;

private:
    UpdatableFeed *m_updatableFeed{nullptr};
    QPointer<LoadOperation> m_operation;

    void onSucceeded(const Syndication::FeedPtr &feed, const QUrl &changeUrl);
    void onFailed(const QString &errorString);
};

Feed::Updater *UpdatableFeed::updater()
{
    return m_updater;
}

FeedCore::UpdatableFeed::UpdatableFeed(QObject *parent)
    : Feed(parent)
    , m_updater{new UpdaterImpl(this, this)}
{
}

static inline bool hasImageUrl(const Syndication::ImagePtr &image)
{
    return !(image.isNull() || image->url().isEmpty());
}

static inline QUrl getIconUrl(const Syndication::FeedPtr &feed, const QUrl &feedUrl)
{
    const Syndication::ImagePtr icon = feed->icon();
    if (hasImageUrl(icon)) {
        return feedUrl.resolved(icon->url());
    }

    const Syndication::ImagePtr image = feed->image();
    if (hasImageUrl(image)) {
        return feedUrl.resolved(image->url());
    }

    return QUrl();
}

void UpdatableFeed::updateFromSource(const Syndication::FeedPtr &feed)
{
    if (name().isEmpty()) {
        setName(feed->title());
    }
    setLink(feed->link());
    setIcon(getIconUrl(feed, url()));
    const auto &items = feed->items();
    time_t expireTime = 0;
    if ((expireAge() > 0) && (expireMode() != DisableUpdateMode)) {
        expireTime = updater()->updateStartTime().toTime_t() - expireAge();
    }
    for (const auto &item : items) {
        const auto &dateUpdated = item->dateUpdated();
        if (dateUpdated == 0 || dateUpdated >= expireTime) {
            updateSourceArticle(item);
        }
    }
    if (expireTime > 0) {
        expire(QDateTime::fromTime_t(expireTime));
    }
}

UpdatableFeed::UpdaterImpl::UpdaterImpl(UpdatableFeed *feed, QObject *parent)
    : Updater(feed, parent)
    , m_updatableFeed{feed}
{
}

void UpdatableFeed::UpdaterImpl::run()
{
    if (!feed()->url().isValid()) {
        setError(tr("Invalid URL", "error message"));
        return;
    }
    m_operation = new LoadOperation;
    QObject::connect(m_operation, &LoadOperation::succeeded, this, &UpdaterImpl::onSucceeded);
    QObject::connect(m_operation, &LoadOperation::failed, this, &UpdaterImpl::onFailed);
    QObject::connect(m_operation, &LoadOperation::aborted, this, [this] {
        aborted();
    });
    m_operation->start(feed()->url());
}

void UpdatableFeed::UpdaterImpl::abort()
{
    if (!m_operation.isNull()) {
        m_operation->abort();
    }
}

void UpdatableFeed::UpdaterImpl::onSucceeded(const Syndication::FeedPtr &feed, const QUrl &changeUrl)
{
    if (changeUrl.isValid()) {
        m_updatableFeed->setUrl(changeUrl);
    }
    m_updatableFeed->updateFromSource(feed);
    finish();
}

void UpdatableFeed::UpdaterImpl::onFailed(const QString &errorString)
{
    qDebug() << "Updater Error:" << errorString;
    setError(errorString);
}

void LoadOperation::start(const QUrl &url, const QString &failMessage)
{
    if (m_seenUrls.contains(url) || m_seenUrls.count() > kMaxRedirects) {
        const QString &errorMessage = failMessage.isEmpty() ? "unknown error" : failMessage;
        emit failed(errorMessage, DeleteLater(this));
    }
    m_seenUrls << url;
    QNetworkRequest request(url);
    m_reply = NetworkAccessManager::instance()->get(request);
    QObject::connect(m_reply, &QNetworkReply::finished, this, &LoadOperation::onReplyFinished);
}

void LoadOperation::abort()
{
    m_reply->abort();
}

void LoadOperation::onReplyFinished()
{
    m_reply->deleteLater();
    QUrl url = m_reply->url();

    switch (m_reply->error()) {
    case QNetworkReply::NoError: {
        QByteArray data = m_reply->readAll();
        Syndication::FeedPtr feed = Syndication::parserCollection()->parse({data, m_reply->url().toString()});
        if (feed != nullptr) {
            QUrl changeUrl = m_isDiscoveredFeed ? url : QUrl();
            emit succeeded(feed, changeUrl, DeleteLater(this));
        } else {
            QUrl discoveredFeed = FeedDiscovery::discoverFeed(url, data);
            m_isDiscoveredFeed = true;
            start(discoveredFeed, "couldn't find feed source");
        }
        break;
    }

    case QNetworkReply::OperationCanceledError:
        emit aborted(DeleteLater(this));
        break;

    case QNetworkReply::InsecureRedirectError: {
        const QUrl redirect = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
        qDebug() << "insecure redirect from" << url << "to" << redirect;
        start(redirect, "too many redirects");
        break;
    }

    default:
        emit failed(m_reply->errorString(), DeleteLater(this));
    }
}

#include "updatablefeed.moc"
