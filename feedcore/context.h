﻿#ifndef CONTEXT_H
#define CONTEXT_H
#include <memory>
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "future.h"

class QNetworkAccessManager;
namespace FeedCore {
class Storage;
class Feed;
class ProvisionalFeed;

class Context : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 defaultUpdateInterval READ defaultUpdateInterval WRITE setDefaultUpdateInterval NOTIFY defaultUpdateIntervalChanged)
    Q_PROPERTY(qint64 expireAge READ expireAge WRITE setExpireAge NOTIFY expireAgeChanged)
public:
    explicit Context(Storage *storage, QObject *parent = nullptr);
    ~Context();
    Future<Feed*> *getFeeds();
    Q_INVOKABLE void addFeed(FeedCore::Feed *feed);
    Future<ArticleRef> *getArticles(bool unreadFilter);
    Future<ArticleRef> *getStarred();
    void requestUpdate();
    void abortUpdates();
    qint64 defaultUpdateInterval();
    void setDefaultUpdateInterval(qint64 defaultUpdateInterval);
    qint64 expireAge();
    void setExpireAge(qint64 expireAge);
    QNetworkAccessManager *networkAccessManager() const;
signals:
    void defaultUpdateIntervalChanged();
    void expireAgeChanged();
    void feedAdded(FeedCore::Feed *feed);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};
}
#endif // CONTEXT_H
