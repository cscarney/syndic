#pragma once

#include "articlelistmodel.h"

namespace FeedCore
{
class Context;
}

class OverviewModel : public ArticleListModel
{
    Q_OBJECT
    Q_PROPERTY(FeedCore::Context *context READ context WRITE context NOTIFY contextChanged)
public:
    explicit OverviewModel(QObject *parent = nullptr);
    FeedCore::Context *context() const;
    void context(FeedCore::Context *newContext);
signals:
    void contextChanged();

private:
    FeedCore::Context *m_context = nullptr;

    // ArticleListModel interface
protected:
    QFuture<FeedCore::ArticleRef> getArticles() override;

    // ArticleListModel interface
public:
    void requestUpdate() override;
};
