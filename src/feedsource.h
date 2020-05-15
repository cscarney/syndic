#ifndef FEEDSOURCE_H
#define FEEDSOURCE_H

#include <QObject>
#include <QNetworkAccessManager>

#include <optional>
#include <functional>

class FeedSource : public QObject
{
    Q_OBJECT
public:
    struct Item {
        qint64 id;
        qint64 feedId;
        QString headline;
        QString author;
        QDateTime date;
        QString content;
        QString url;
        bool isUnread;
        bool isStarred;
    };

    struct Feed {
        qint64 id;
        QString name;
    };

    enum ItemFlag {
        NONE = 0,
        UNREAD = 1,
        STARRED = 1<<1
    };

    explicit FeedSource(QObject *parent = nullptr) : QObject(parent) {};
    virtual void beginUpdate() = 0;

    virtual void setFlag(qint64 id, ItemFlag flag, bool state) {}
    virtual void addFeed(const QUrl&) {}

signals:
    void foundFeed(Feed const &feed);
    void foundContent(Item const &item);
    void updateError(QString errorMessage);

    void feedRead(qint64 feedId, QDateTime olderThan=QDateTime());
};

#endif // FEEDSOURCE_H
