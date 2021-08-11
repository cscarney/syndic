/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Syndication/Image>
#include <Syndication/Item>
#include <Syndication/Person>
#include "provisionalfeed.h"
#include "updater.h"
#include "article.h"
using namespace FeedCore;

class ProvisionalFeed::ArticleImpl : public Article {
public:
    explicit ArticleImpl(const Syndication::ItemPtr& item, Feed *feed, QObject *parent=nullptr);
    void requestContent() final;
    void setRead(bool isRead) final {};
    void setStarred(bool isStarred) final {};
private:
    Syndication::ItemPtr m_item;
};

void ProvisionalFeed::onUrlChanged() {
    updater()->abort();
    m_feed = nullptr;
    emit reset();
}

ProvisionalFeed::ProvisionalFeed(QObject *parent) :
    UpdatableFeed(parent)
{
    QObject::connect(this, &Feed::urlChanged, this, &ProvisionalFeed::onUrlChanged);
    QObject::connect(this, &ProvisionalFeed::targetFeedChanged, this,
                     [this]{ updateParams(m_targetFeed); });
}

Future<ArticleRef> *ProvisionalFeed::getArticles(bool unreadFilter)
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        if (m_feed == nullptr) {return;}
        const auto &items = m_feed->items();
        for(const auto &item : items) {
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

void ProvisionalFeed::save()
{
    if (m_targetFeed == nullptr){return;}
    m_targetFeed->updateParams(this);
}

void ProvisionalFeed::ArticleImpl::requestContent()
{
    QString content { m_item->content() };
    emit gotContent(content.isEmpty() ? m_item->description() : content);
}

ProvisionalFeed::ArticleImpl::ArticleImpl(const Syndication::ItemPtr& item, Feed *feed, QObject *parent):
    Article(feed, parent),
    m_item(item)
{
    setTitle(item->title());
    setUrl(item->link());
    setDate(QDateTime::fromTime_t(item->dateUpdated()));
    auto authors = item->authors();
    setAuthor(authors.isEmpty() ? "" : authors[0]->name());
}

