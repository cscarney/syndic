#ifndef CONTEXT_H
#define CONTEXT_H
#include <memory>
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "future.h"

namespace FeedCore {
class Storage;
class FeedRef;

class Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(Storage *storage, QObject *parent = nullptr);
    ~Context();
    Future<FeedRef> *getFeeds();
    Q_INVOKABLE void addFeed(const QUrl &url);
    Future<ArticleRef> *getArticles(bool unreadFilter);
    void requestUpdate();
    bool updatesInProgress();
signals:
    void feedAdded(const FeedCore::FeedRef &feed);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};
}
#endif // CONTEXT_H
