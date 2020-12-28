#ifndef FEED_H
#define FEED_H
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>

#include "enums.h"
#include "feedstorageoperation.h"
#include "articleref.h"

namespace FeedCore {

class FeedUpdater;

class Feed : public QObject {
    Q_OBJECT

public:
    QString name() const;
    virtual void setName(const QString &name) { populateName(name); }
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);

    QUrl url() const;
    void setUrl(const QUrl &url);
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);

    int unreadCount() const;
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);

    LoadStatus status() const;
    void setStatus(LoadStatus status);
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);

    virtual FeedUpdater *updater() = 0;
    Q_PROPERTY(FeedUpdater *updater READ updater CONSTANT);

    virtual ItemQuery *startItemQuery(bool unreadFilter)=0;
    virtual void updateFromSource(const Syndication::FeedPtr &feed){ assert(false); };

signals:
    void nameChanged();
    void urlChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void itemAdded(const FeedCore::ArticleRef &item);

protected:
    explicit Feed(QObject *parent = nullptr);
    bool populateName(const QString &name);
    void populateUrl(const QUrl &url);
    void populateUnreadCount(int unreadCount);
    void incrementUnreadCount(int delta=1);
    inline void decrementUnreadCount() { incrementUnreadCount(-1); };

private:
    QString m_name;
    QUrl m_url;
    int m_unreadCount = 0;
    LoadStatus m_status = LoadStatus::Idle;
};

}

#endif // STOREDFEED_H
