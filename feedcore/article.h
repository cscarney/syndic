#ifndef ARTICLE_H
#define ARTICLE_H
#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QPointer>

namespace FeedCore {
class Feed;
class Article : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FeedCore::Feed *feed READ feed CONSTANT)
    Q_PROPERTY(bool isRead READ isRead WRITE setRead NOTIFY readStatusChanged);
    Q_PROPERTY(bool isStarred READ isStarred WRITE setStarred NOTIFY starredChanged);
    Q_PROPERTY(QString title READ title NOTIFY titleChanged);
    Q_PROPERTY(QString author READ author NOTIFY authorChanged);
    Q_PROPERTY(QDateTime date READ date NOTIFY dateChanged);
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);
public:
    Feed *feed() const;
    const QString &title() const { return m_title; }
    const QString &author() const { return m_author; }
    const QDateTime &date() const { return m_date; }
    const QUrl &url() const { return m_url; }
    bool isRead() const { return m_readStatus; }
    virtual void setRead(bool isRead);
    bool isStarred() const { return m_starred; }
    virtual void setStarred(bool isStarred);
    Q_INVOKABLE virtual void requestContent() = 0;
signals:
    void titleChanged();
    void authorChanged();
    void dateChanged();
    void urlChanged();
    void readStatusChanged();
    void starredChanged();
    void gotContent(const QString &content);
protected:
    explicit Article(Feed *feed, QObject *parent = nullptr);
    void setTitle(const QString &title);
    void setAuthor(const QString &author);
    void setDate(const QDateTime &date);
    void setUrl(const QUrl &url);
private:
    QPointer<Feed> m_feed;
    QString m_title;
    QString m_author;
    QDateTime m_date;
    QUrl m_url;
    bool m_readStatus { false };
    bool m_starred { false };
};
}

#endif // ARTICLE_H
