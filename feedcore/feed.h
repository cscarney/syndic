#ifndef FEED_H
#define FEED_H
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "enums.h"
#include "future.h"

namespace FeedCore {
class Updater;

class Feed : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged);
    Q_PROPERTY(QUrl link READ link WRITE setLink NOTIFY linkChanged);
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);
    Q_PROPERTY(FeedCore::Updater *updater READ updater CONSTANT);
    Q_PROPERTY(bool editable READ editable CONSTANT);
public:
    ~Feed();
    const QString &name() const { return m_name; }
    void setName(const QString &name);
    const QUrl &url() const { return m_url; }
    void setUrl(const QUrl &url);
    const QUrl &link() { return m_link; }
    void setLink(const QUrl &link);
    int unreadCount() const;
    LoadStatus status() const;
    void setStatus(LoadStatus status);
    void updateParams(Feed *other);
    virtual Updater *updater() = 0;
    virtual Future<ArticleRef> *getArticles(bool unreadFilter)=0;
    virtual bool editable() { return false; }
    virtual void updateFromSource(const Syndication::FeedPtr &feed){ assert(false); };
    Q_INVOKABLE virtual void requestDelete(){ emit deleteRequested(); }
signals:
    void nameChanged();
    void urlChanged();
    void linkChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void articleAdded(const FeedCore::ArticleRef &article);
    void reset();
    void deleteRequested();
protected:
    explicit Feed(QObject *parent = nullptr);
    void setUnreadCount(int unreadCount);
    void incrementUnreadCount(int delta=1);
    void decrementUnreadCount() { incrementUnreadCount(-1); };
private:
    QString m_name;
    QUrl m_url;
    QUrl m_link;
    int m_unreadCount { 0 };
    LoadStatus m_status { LoadStatus::Idle };
};
}
#endif // FEED_H
