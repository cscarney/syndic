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
    Q_PROPERTY(qint64 defaultUpdateInterval READ defaultUpdateInterval WRITE setDefaultUpdateInterval NOTIFY defaultUpdateIntervalChanged)
public:
    explicit Context(Storage *storage, QObject *parent = nullptr);
    ~Context();
    Future<FeedRef> *getFeeds();
    Q_INVOKABLE void addFeed(FeedCore::Feed *feed);
    Future<ArticleRef> *getArticles(bool unreadFilter);
    void requestUpdate();
    void abortUpdates();
    qint64 defaultUpdateInterval();
    void setDefaultUpdateInterval(qint64 defaultUpdateInterval);
signals:
    void defaultUpdateIntervalChanged();
    void feedAdded(const FeedCore::FeedRef &feed);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};
}
#endif // CONTEXT_H
