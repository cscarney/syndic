#pragma once

#include "context.h"
#include "feed.h"

namespace FeedCore
{

class SearchResultFeed : public FeedCore::Feed
{
    Q_OBJECT

    Q_PROPERTY(Context *context READ context WRITE setContext NOTIFY contextChanged FINAL)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged FINAL)
public:
    explicit SearchResultFeed(QObject *parent = nullptr);

    QFuture<ArticleRef> getArticles(bool unreadFilter) override;
    Updater *updater() override;
    Context *context() const;
    void setContext(Context *newContext);
    QString query() const;
    void setQuery(const QString &newQuery);

signals:
    void contextChanged();
    void queryChanged();

private:
    class Updater;
    Context *m_context;
    QString m_query;
    Updater *m_updater{nullptr};
};

} // namespace FeedCore
