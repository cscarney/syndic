#ifndef CONTEXT_H
#define CONTEXT_H
#include <memory>
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "future.h"
#include "feedref.h"

namespace FeedCore {
class Storage;
class ProvisionalFeed;

class Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(Storage *storage, QObject *parent = nullptr);
    ~Context();
    Future<FeedRef> *getFeeds();
    Q_INVOKABLE void addFeed(ProvisionalFeed *feed);
    Future<ArticleRef> *getArticles(bool unreadFilter);
    void requestUpdate();
    void abortUpdates();
signals:
    void feedAdded(const FeedCore::FeedRef &feed);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};
}
#endif // CONTEXT_H
