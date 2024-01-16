#include "highlightsmodel.h"
#include "context.h"

using namespace FeedCore;

HighlightsModel::HighlightsModel(QObject *parent)
    : ArticleListModel{parent}
{
}

Context *HighlightsModel::context() const
{
    return m_context;
}

void HighlightsModel::context(Context *newContext)
{
    if (m_context == newContext)
        return;
    m_context = newContext;
    m_waitForFeed = newContext->allItemsFeed();
    emit contextChanged();
}

static bool feedIsReady(const QSharedPointer<Feed> &feed)
{
    return feed->status() != Feed::Loading && feed->status() != Feed::Updating;
}

QFuture<ArticleRef> HighlightsModel::getArticles()
{
    if (!m_context)
        return QFuture<ArticleRef>{};

    if (m_waitForFeed && !feedIsReady(m_waitForFeed)) {
        // wait for load to finish
        return QtFuture::connect(m_waitForFeed.get(), &Feed::statusChanged)
            .then(this,
                  [this] {
                      return getArticles();
                  })
            .unwrap();
    }

    m_waitForFeed.clear();
    return m_context->getHighlights();
}

void HighlightsModel::requestUpdate()
{
    refresh();
}

void HighlightsModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent)) {
        return;
    }

    qDebug() << "fetching more";
    refreshMerge();
}

bool HighlightsModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }

    if (status() != Feed::Idle) {
        return false;
    }

    return true;
}
