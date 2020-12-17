#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <memory>

#include <QObject>
#include <QUrl>
#include <Syndication/Feed>

#include "enums.h"
#include "feedref.h"
#include "feedstorageoperation.h"

namespace FeedCore {

class FeedUpdater;

class Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    ~Context();

    FeedQuery *startFeedQuery();

    Q_INVOKABLE void addFeed(const QUrl &url);

    ItemQuery *startQuery(bool unreadFilter);
    void requestUpdate();
    void requestUpdate(const FeedRef &feed);
    bool updatesInProgress();

signals:
    void feedAdded(const FeedCore::FeedRef &feed);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};

}

#endif // FEEDMANAGER_H
