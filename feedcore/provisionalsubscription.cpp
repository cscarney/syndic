/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "provisionalsubscription.h"
#include "article.h"
#include <QDir>
#include <Syndication/Image>
#include <Syndication/Item>
#include <Syndication/Person>
using namespace FeedCore;

class ProvisionalSubscription::ArticleImpl : public Article
{
public:
    explicit ArticleImpl(const Syndication::ItemPtr &item, Feed *feed, QObject *parent = nullptr);
    void requestContent() final;
    void setRead(bool /*isRead*/) final{};
    void setStarred(bool /*isStarred*/) final{};

private:
    Syndication::ItemPtr m_item;
};

void ProvisionalSubscription::onUrlChanged()
{
    syncUrlString();
    cancelUpdates();
    m_feed = nullptr;
    setFlags(flags() & ~Subscription::IsWebPageFlag);
    emit reset();
}

ProvisionalSubscription::ProvisionalSubscription(QObject *parent)
    : LocalSubscription(parent)
{
    syncUrlString();
    QObject::connect(this, &Subscription::urlChanged, this, &ProvisionalSubscription::onUrlChanged);
    QObject::connect(this, &ProvisionalSubscription::targetFeedChanged, this, [this] {
        updateParams(m_targetFeed);
    });
}

QFuture<ArticleRef> ProvisionalSubscription::getArticles(bool /* unreadFilter */)
{
    return Future::yield<ArticleRef>(this, [this](auto &op) {
        if (m_feed == nullptr) {
            return;
        }
        const auto &items = m_feed->items();
        for (const auto &item : items) {
            op.addResult(m_articles.getInstance(item, this));
        }
    });
}

QFuture<void> ProvisionalSubscription::updateFromSource(const Syndication::FeedPtr &feed)
{
    if (name().isEmpty()) {
        setName(feed->title());
    }
    setLink(feed->link());
    if (auto icon = feed->icon()) {
        setIcon(icon->url());
    }
    setUnreadCount(feed->items().size());
    m_feed = feed;
    emit reset();
    return QtFuture::makeReadyVoidFuture();
}

QFuture<void> ProvisionalSubscription::updateSourceArticle(const Syndication::ItemPtr &)
{
    return QtFuture::makeReadyVoidFuture();
}

Subscription *ProvisionalSubscription::targetFeed() const
{
    return m_targetFeed;
}

void ProvisionalSubscription::setTargetFeed(Subscription *targetFeed)
{
    if (m_targetFeed == targetFeed) {
        return;
    }
    m_targetFeed = targetFeed;
    updateParams(m_targetFeed);
    emit targetFeedChanged();
}

void ProvisionalSubscription::save()
{
    if (m_targetFeed == nullptr) {
        return;
    }
    m_targetFeed->updateParams(this);
}

void ProvisionalSubscription::ArticleImpl::requestContent()
{
    QString content{m_item->content()};
    emit gotContent(content.isEmpty() ? m_item->description() : content);
}

ProvisionalSubscription::ArticleImpl::ArticleImpl(const Syndication::ItemPtr &item, Feed *feed, QObject *parent)
    : Article(feed, parent)
    , m_item(item)
{
    setTitle(item->title());
    setUrl(item->link());
    setDate(QDateTime::fromSecsSinceEpoch(item->dateUpdated()));
    auto authors = item->authors();
    setAuthor(authors.isEmpty() ? "" : authors[0]->name());
}

const QString &ProvisionalSubscription::urlString() const
{
    return m_urlString;
}

// This is similar to QUrl::fromUserInput, but we need slightly different behavior
static QUrl urlFromString(const QString &string)
{
    QString trimmedString = string.trimmed();

    if (trimmedString.isEmpty()) {
        return QUrl();
    }

    if (QDir::isAbsolutePath(string)) {
        return QUrl::fromLocalFile(string);
    }

    QUrl url(trimmedString, QUrl::TolerantMode);
    if (url.isValid() && !url.isRelative()) {
        return url;
    }

    QUrl httpsUrl(QLatin1String("https://") + trimmedString, QUrl::TolerantMode);
    if (httpsUrl.isValid()) {
        return httpsUrl;
    }

    return QUrl();
}

void ProvisionalSubscription::setUrlString(const QString &newUrlString)
{
    if (m_urlStringStatus == PENDING) {
        return;
    }
    if (m_urlString == newUrlString) {
        return;
    }
    m_urlStringStatus = PENDING;
    m_urlString = newUrlString;
    QUrl url = urlFromString(newUrlString);
    if (url.isValid()) {
        setUrl(url);
        m_urlStringStatus = VALID;
    } else {
        m_urlStringStatus = INVALID;
    }
    emit urlStringChanged();
    emit urlStringEdited();
}

void ProvisionalSubscription::syncUrlString()
{
    if (m_urlStringStatus == PENDING) {
        return;
    }
    const QUrl &url = this->url();
    const QString urlString = url.toString();
    if (m_urlString == urlString) {
        return;
    }
    m_urlString = urlString;
    m_urlStringStatus = url.isValid() ? VALID : INVALID;
    emit urlStringChanged();
}
