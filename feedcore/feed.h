#ifndef FEED_H
#define FEED_H
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "enums.h"
#include "future.h"

namespace FeedCore {
class Updater;

/**
 * Abstract class for feeds
 *
 * This represents a monitored/stored feed in the application.  The content
 * of a remote feed is represented by Syndication::Feed (from the KSyndication library).
 */
class Feed : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged);
    Q_PROPERTY(QUrl link READ link WRITE setLink NOTIFY linkChanged);
    Q_PROPERTY(QUrl icon READ icon WRITE setIcon NOTIFY iconChanged);
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);
    Q_PROPERTY(FeedCore::Updater *updater READ updater CONSTANT);
    Q_PROPERTY(bool editable READ editable CONSTANT);
public:
    ~Feed();
    /**
     * The user-facing display name of the feed.
     */
    const QString &name() const { return m_name; }

    /**
     * Set the name of the feed.
     */
    void setName(const QString &name);

    /**
     * The url of the feed source.
     */
    const QUrl &url() const { return m_url; }

    /**
     * Set the url of the feed source.
     */
    void setUrl(const QUrl &url);

    /**
     * A link to a web page associated with this feed
     */
    const QUrl &link() { return m_link; }

    /**
     * Set the web link associated associated with this feed
     */
    void setLink(const QUrl &link);

    /**
     * A URL pointing to an image that will be displayed as the feed's icon
     */
    const QUrl &icon() { return m_icon; }

    /**
     * Set the icon URL.
     */
    void setIcon(const QUrl &icon);

    /**
     * The number of unread articles associated with this feed.
     */
    int unreadCount() const;

    /**
     * The feed's update status
     */
    LoadStatus status() const;

    /**
     * Set the update status.
     *
     * This should probably only be called by an Updater implementation.
     */
    void setStatus(LoadStatus status);

    /**
     * Set this feed's metadata to match that of /other/
     */
    void updateParams(Feed *other);

    /**
     * The Updater instance that should be used to update this feed.
     *
     * The returned updater belongs to the Feed object.
     *
     * Derived classes can provide their own Updater implementation,
     * or implement updateFromSource and return an XMLUpdater instance.
     */
    virtual Updater *updater() = 0;

    /**
     * Returns a future representing all of the stored articles associated with this feed.
     *
     * If unreadFilter is true, only unread articles are returned.
     */
    virtual Future<ArticleRef> *getArticles(bool unreadFilter)=0;

    /**
     * Returns true if the implementation supports storing and propagating
     * changes to the feed's properties.
     */
    virtual bool editable() { return false; }

    /**
     * Update the stored feed from a Syndication::Feed object.
     *
     * Derived classes must implement this if they return an XMLUpdater
     * instance from updater()
     */
    virtual void updateFromSource(const Syndication::FeedPtr &feed){ assert(false); };

    /**
     * Request that the feed be deleted from the storage backend.
     *
     * If the delete operation succeeds, the Feed object will be
     * destroyed, possibly asynchronously.  Connect to the QObject::destroyed
     * signal on the feed to handle this.
     */
    Q_INVOKABLE virtual void requestDelete(){ emit deleteRequested(); }
signals:
    void nameChanged();
    void urlChanged();
    void linkChanged();
    void iconChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void articleAdded(const FeedCore::ArticleRef &article);

    /**
     * Emitted when a feed has changed significantly (e.g. when a
     * ProvisionalFeed is pointed at a new source). Code that
     * depends on the state of the Feed object should re-sync.
     */
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
    QUrl m_icon;
    int m_unreadCount { 0 };
    LoadStatus m_status { LoadStatus::Idle };
};
}
#endif // FEED_H
