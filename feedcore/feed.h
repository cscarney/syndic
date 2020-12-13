#ifndef STOREDFEED_H
#define STOREDFEED_H

#include <QObject>
#include <QUrl>
#include <Syndication/Feed>

namespace FeedCore {

class StoredItem;

class Feed : public QObject {
    Q_OBJECT

public:
    QString name() const;
    void setName(const QString &s);
    Q_PROPERTY(QString name READ name NOTIFY nameChanged);

    QUrl url() const;
    void setUrl(const QUrl &url);
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);

    int unreadCount() const;
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);

signals:
    void nameChanged();
    void urlChanged();
    void unreadCountChanged();
    void itemAdded(const FeedCore::StoredItem &item);

protected:
    Feed(QObject *parent = nullptr);
    void populateName(const QString &name);
    void populateUrl(const QUrl &url);
    void populateUnreadCount(int unreadCount);

private:
    QString m_name;
    QUrl m_url;
    int m_unreadCount = 0;
};

}

#endif // STOREDFEED_H
