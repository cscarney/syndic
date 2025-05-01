/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "updatablefeed.h"
#include "article.h"
#include "articlelinkextractor.h"
#include "context.h"
#include "feeddiscovery.h"
#include "networkaccessmanager.h"
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>
#include <QQueue>
#include <Syndication/Image>
#include <Syndication/ParserCollection>
using namespace FeedCore;

constexpr const int kMaxRedirects = 10;

namespace
{
class DeleteLater
{
public:
    void operator()(QObject *o)
    {
        o->deleteLater();
    }
};

class LoadOperation : public QObject
{
    Q_OBJECT
public:
    void start(const QUrl &url, const QString &failMessage = QString());
    void abort();

signals:
    void succeeded(const QByteArray &feed, const QUrl &changeUrl);
    void failed(const QString &errorString);
    void aborted();

private:
    QSet<QUrl> m_seenUrls;
    QPointer<QNetworkReply> m_reply;
    void onReplyFinished();
};

class Update : public QObject
{
    Q_OBJECT
public:
    explicit Update(const UpdatableFeed *feed);
    void abort();
    void start();

signals:
    void succeeded(const Syndication::FeedPtr &feed);
    void failed(const QString &errorString);
    void aborted();

private:
    UpdatableFeed *m_feed{nullptr};
    std::unique_ptr<LoadOperation, DeleteLater> m_currentOperation;
    QByteArray m_firstData;

    void onPrimaryFeedFetchSucceeded(const QByteArray &data, const QUrl &url);
    void onWebPageFetchSucceeded(const QByteArray &data, const QUrl &url);
    void onDiscoveredFeedFetchSucceeded(const QByteArray &data, const QUrl &url);
    void onDiscoveredFeedFetchFailed(const QString &errorString);
    void fallbackToWebPage();
    void onFailed(const QString &errorString);
    void onAborted();
};

}

class UpdatableFeed::UpdaterImpl : public Feed::Updater
{
public:
    UpdaterImpl(UpdatableFeed *feed, QObject *parent);
    void run() final;
    void abort() final;
    void cleanup() final;

private:
    UpdatableFeed *m_updatableFeed{nullptr};
    std::unique_ptr<Update> m_currentUpdate;

    void onSucceeded(const Syndication::FeedPtr &feed);
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

QFuture<void> UpdatableFeed::updateFromSource(const Syndication::FeedPtr &feed)
{
    if (name().isEmpty()) {
        setName(feed->title());
    }
    setLink(feed->link());
    setIcon(getIconUrl(feed, url()));
    const auto &items = feed->items();
    time_t expireTime = 0;
    if ((expireAge() > 0) && (expireMode() != DisableUpdateMode)) {
        expireTime = updater()->updateStartTime().toSecsSinceEpoch() - expireAge();
    }
    QList<QFuture<void>> addResults;
    for (const auto &item : items) {
        const auto &dateUpdated = item->dateUpdated();
        if (dateUpdated == 0 || dateUpdated >= expireTime) {
            addResults << updateSourceArticle(item);
        }
    }
    if (expireTime > 0) {
        expire(QDateTime::fromSecsSinceEpoch(expireTime));
    }

    return QtFuture::whenAll(addResults.begin(), addResults.end()).then([](auto) {});
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
    m_currentUpdate.reset(new Update(m_updatableFeed));
    QObject::connect(m_currentUpdate.get(), &Update::succeeded, this, &UpdaterImpl::onSucceeded);
    QObject::connect(m_currentUpdate.get(), &Update::failed, this, &UpdaterImpl::onFailed);
    QObject::connect(m_currentUpdate.get(), &Update::aborted, this, &UpdaterImpl::aborted);
    m_currentUpdate->start();
}

void UpdatableFeed::UpdaterImpl::abort()
{
    if (m_currentUpdate) {
        m_currentUpdate->abort();
    }
}

void UpdatableFeed::UpdaterImpl::cleanup()
{
    if (auto *update = m_currentUpdate.release()) {
        update->deleteLater();
    }
}

void UpdatableFeed::UpdaterImpl::onSucceeded(const Syndication::FeedPtr &feed)
{
    auto whenDone = m_updatableFeed->updateFromSource(feed);
    Future::safeThen(whenDone, this, [this](auto) {
        finish();
    });
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
        emit failed(errorMessage);
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
        emit succeeded(data, url);
        break;
    }

    case QNetworkReply::OperationCanceledError:
        emit aborted();
        break;

    case QNetworkReply::InsecureRedirectError: {
        const QUrl redirect = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
        qDebug() << "insecure redirect from" << url << "to" << redirect;
        start(redirect, "too many redirects");
        break;
    }

    default:
        emit failed(m_reply->errorString());
    }
}

Update::Update(const UpdatableFeed *feed)
    : m_feed{const_cast<UpdatableFeed *>(feed)}
{
}

void Update::abort()
{
    m_currentOperation->abort();
}

void Update::start()
{
    m_currentOperation.reset(new LoadOperation);
    if (m_feed->flags() & Feed::IsWebPageFlag) {
        QObject::connect(m_currentOperation.get(), &LoadOperation::succeeded, this, &Update::onWebPageFetchSucceeded);
    } else {
        QObject::connect(m_currentOperation.get(), &LoadOperation::succeeded, this, &Update::onPrimaryFeedFetchSucceeded);
    }
    QObject::connect(m_currentOperation.get(), &LoadOperation::failed, this, &Update::onFailed);
    QObject::connect(m_currentOperation.get(), &LoadOperation::aborted, this, &Update::onAborted);
    m_currentOperation->start(m_feed->url());
}

void Update::onPrimaryFeedFetchSucceeded(const QByteArray &data, const QUrl &url)
{
    m_firstData = data;
    Syndication::FeedPtr feed = Syndication::parserCollection()->parse({data, url.toString()});
    if (feed.isNull()) {
        // if the feed didn't parse, try feed discovery
        QUrl discoveredFeedUrl = FeedDiscovery::discoverFeed(m_feed->url(), data);
        m_currentOperation.reset(new LoadOperation);
        QObject::connect(m_currentOperation.get(), &LoadOperation::succeeded, this, &Update::onDiscoveredFeedFetchSucceeded);
        QObject::connect(m_currentOperation.get(), &LoadOperation::failed, this, &Update::onDiscoveredFeedFetchFailed);
        QObject::connect(m_currentOperation.get(), &LoadOperation::aborted, this, &Update::onAborted);
        m_currentOperation->start(discoveredFeedUrl);
    } else {
        emit succeeded(feed);
    }
}

void Update::onWebPageFetchSucceeded(const QByteArray &data, const QUrl &url)
{
    ArticleLinkExtractor extractor(data, url);
    extractor.walk();
    Syndication::FeedPtr feed = extractor.articleLinksFeed();
    emit succeeded(feed);
}

void Update::onDiscoveredFeedFetchSucceeded(const QByteArray &data, const QUrl &url)
{
    Syndication::FeedPtr feed = Syndication::parserCollection()->parse({data, url.toString()});
    if (feed.isNull()) {
        fallbackToWebPage();
    } else {
        m_feed->setUrl(url);
        emit succeeded(feed);
    }
}

void Update::onDiscoveredFeedFetchFailed(const QString &errorString)
{
    Q_UNUSED(errorString);
    fallbackToWebPage();
}

void Update::fallbackToWebPage()
{
    ArticleLinkExtractor extractor(m_firstData, m_feed->url());
    extractor.walk();
    if (m_feed) {
        m_feed->setFlags(m_feed->flags() | Feed::IsWebPageFlag);
    }
    Syndication::FeedPtr feed = extractor.articleLinksFeed();
    emit succeeded(feed);
    return;
}

void Update::onFailed(const QString &errorString)
{
    emit failed(errorString);
}

void Update::onAborted()
{
    emit aborted();
}

#include "updatablefeed.moc"
