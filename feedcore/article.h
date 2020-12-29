#ifndef ARTICLE_H
#define ARTICLE_H
#include <QObject>
#include <QDateTime>
#include <QUrl>
#include "feedref.h"

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
    const QString &title() const;
    const QString &author() const;
    const QDateTime &date() const;
    const QUrl &url() const;
    bool isRead() const;
    virtual void setRead(bool isRead) = 0;
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
    void populateTitle(const QString &title);
    void populateAuthor(const QString &author);
    void populateDate(const QDateTime &date);
    void populateUrl(const QUrl &url);
    void populateReadStatus(bool isRead);
private:
    QString m_title;
    QString m_author;
    QDateTime m_date;
    QUrl m_url;
    QString m_content;
    bool m_readStatus { false };
};
}

#endif // ARTICLE_H
