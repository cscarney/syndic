/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "provisionalfeed.h"
#include "article.h"
#include <QDir>
#include <Syndication/Image>
#include <Syndication/Item>
#include <Syndication/Person>
using namespace FeedCore;

class ProvisionalFeed::ArticleImpl : public Article
{
public:
    explicit ArticleImpl(const Syndication::ItemPtr &item, Feed *feed, QObject *parent = nullptr);
    void requestContent() final;
    void setRead(bool /*isRead*/) final{};
    void setStarred(bool /*isStarred*/) final{};

private:
    Syndication::ItemPtr m_item;
};

void ProvisionalFeed::onUrlChanged()
{
    syncUrlString();
    updater()->abort();
    m_feed = nullptr;
    emit reset();
}

ProvisionalFeed::ProvisionalFeed(QObject *parent)
    : UpdatableFeed(parent)
{
    syncUrlString();
    QObject::connect(this, &Feed::urlChanged, this, &ProvisionalFeed::onUrlChanged);
    QObject::connect(this, &ProvisionalFeed::targetFeedChanged, this, [this] {
        updateParams(m_targetFeed);
    });
}

Future<ArticleRef> *ProvisionalFeed::getArticles(bool /* unreadFilter */)
{
    return Future<ArticleRef>::yield(this, [this](auto *op) {
        if (m_feed == nullptr) {
            return;
        }
        const auto &items = m_feed->items();
        for (const auto &item : items) {
            op->appendResult(m_articles.getInstance(item, this));
        }
    });
}

void ProvisionalFeed::updateFromSource(const Syndication::FeedPtr &feed)
{
    if (name().isEmpty()) {
        setName(feed->title());
    }
    setLink(feed->link());
    setIcon(feed->icon()->url());
    setUnreadCount(feed->items().size());
    m_feed = feed;
    emit reset();
}

Feed *ProvisionalFeed::targetFeed() const
{
    return m_targetFeed;
}

void ProvisionalFeed::setTargetFeed(Feed *targetFeed)
{
    if (m_targetFeed == targetFeed) {
        return;
    }
    m_targetFeed = targetFeed;
    updateParams(m_targetFeed);
    emit targetFeedChanged();
}

void ProvisionalFeed::save()
{
    if (m_targetFeed == nullptr) {
        return;
    }
    m_targetFeed->updateParams(this);
}

void ProvisionalFeed::ArticleImpl::requestContent()
{
    QString content{m_item->content()};
    emit gotContent(content.isEmpty() ? m_item->description() : content);
}

ProvisionalFeed::ArticleImpl::ArticleImpl(const Syndication::ItemPtr &item, Feed *feed, QObject *parent)
    : Article(feed, parent)
    , m_item(item)
{
    setTitle(item->title());
    setUrl(item->link());
    setDate(QDateTime::fromSecsSinceEpoch(item->dateUpdated()));
    auto authors = item->authors();
    setAuthor(authors.isEmpty() ? "" : authors[0]->name());
}

const QString &ProvisionalFeed::urlString() const
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

void ProvisionalFeed::setUrlString(const QString &newUrlString)
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
}

void ProvisionalFeed::syncUrlString()
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
