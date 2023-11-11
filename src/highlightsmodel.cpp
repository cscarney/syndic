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
    emit contextChanged();
}

QFuture<ArticleRef> HighlightsModel::getArticles()
{
    if (!m_context)
        return QFuture<ArticleRef>{};
    return m_context->getHighlights();
}

void HighlightsModel::requestUpdate()
{
    refresh();
}
