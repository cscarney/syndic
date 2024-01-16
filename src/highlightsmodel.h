#pragma once

#include "articlelistmodel.h"

namespace FeedCore
{
class Context;
}

class HighlightsModel : public ArticleListModel
{
    Q_OBJECT
    Q_PROPERTY(FeedCore::Context *context READ context WRITE context NOTIFY contextChanged)
public:
    explicit HighlightsModel(QObject *parent = nullptr);
    FeedCore::Context *context() const;
    void context(FeedCore::Context *newContext);
    void requestUpdate() override;
    void fetchMore(const QModelIndex &parent) override;
    bool canFetchMore(const QModelIndex &parent) const override;

signals:
    void contextChanged();

protected:
    QFuture<FeedCore::ArticleRef> getArticles() override;

private:
    FeedCore::Context *m_context = nullptr;
    QSharedPointer<FeedCore::Feed> m_waitForFeed;
};
