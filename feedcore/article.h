#ifndef ARTICLE_H
#define ARTICLE_H
#include <QObject>
#include <QDateTime>
#include <QUrl>

namespace FeedCore {
class Article : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRead READ isRead WRITE setRead NOTIFY readStatusChanged);
    Q_PROPERTY(QString title READ title NOTIFY titleChanged);
    Q_PROPERTY(QString author READ author NOTIFY authorChanged);
    Q_PROPERTY(QDateTime date READ date NOTIFY dateChanged);
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);
public:
    const QString &title() const { return m_title; }
    const QString &author() const { return m_author; }
    const QDateTime &date() const { return m_date; }
    const QUrl &url() const { return m_url; }
    bool isRead() const { return m_readStatus; }
    virtual void setRead(bool isRead);
    Q_INVOKABLE virtual void requestContent() = 0;
signals:
    void titleChanged();
    void authorChanged();
    void dateChanged();
    void urlChanged();
    void readStatusChanged();
    void gotContent(const QString &content);
protected:
    explicit Article(QObject *parent = nullptr);
    void setTitle(const QString &title);
    void setAuthor(const QString &author);
    void setDate(const QDateTime &date);
    void setUrl(const QUrl &url);
private:
    QString m_title;
    QString m_author;
    QDateTime m_date;
    QUrl m_url;
    bool m_readStatus { false };
};
}

#endif // ARTICLE_H
