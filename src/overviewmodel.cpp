#include "overviewmodel.h"
#include "context.h"

using namespace FeedCore;

OverviewModel::OverviewModel(QObject *parent)
    : ArticleListModel{parent}
{
}

Context *OverviewModel::context() const
{
    return m_context;
}

void OverviewModel::context(Context *newContext)
{
    if (m_context == newContext)
        return;
    m_context = newContext;
    emit contextChanged();
}

QFuture<ArticleRef> OverviewModel::getArticles()
{
    if (!m_context)
        return QFuture<ArticleRef>{};
    return m_context->getHighlights();
}

void OverviewModel::requestUpdate()
{
    refresh();
}
