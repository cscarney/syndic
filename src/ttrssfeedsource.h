#ifndef TTRSSFEEDSOURCE_H
#define TTRSSFEEDSOURCE_H

#include <functional>
#include <QSettings>
#include "feedsource.h"

class TTRSSFeedSource : public FeedSource
{
    Q_OBJECT;
public:
    explicit TTRSSFeedSource(QObject *parent=nullptr);
    void beginUpdate() override;
    void setFlag(qint64 id, ItemFlag flag, bool state) override;
    void addFeed(const QUrl &) override;

signals:
    void loginSuccess();
    void finished();

private:
    QString m_host;
    QString m_user;
    QString m_password;
    bool m_loginInProgress = false;
    bool m_fullUpdate = false;
    int m_pendingRequests = 0;
    QString m_sessionId;
    QNetworkAccessManager m_nam;
    QSettings m_settings;
    qint64 m_lastId;

    typedef std::function<void(QJsonDocument const &, QJsonObject const &)> RequestCallback;
    typedef void(TTRSSFeedSource::*UnboundRequestCallback)(QJsonDocument const&, QJsonObject const&);
    RequestCallback bindCallback(UnboundRequestCallback member);

    void performRequest(QJsonObject const &requestData, const RequestCallback& callback);
    void performAuthenticatedRequest(QJsonObject const &requestData, RequestCallback const &callback = nullptr);
    void performAuthenticatedRequest(QJsonObject const &requestData, UnboundRequestCallback callback);
    void gotAuthenticatedReply(QJsonDocument const &replyDoc, QJsonObject const &requestData, RequestCallback const &callback);
    void dispatchResult(RequestCallback const &callback, QJsonDocument const &doc, QJsonObject const &request);

    void beginLogin();
    void gotLogin(QJsonDocument const &doc, QJsonObject const &request);

    void getHeadlines(qint64 sinceId);
    void gotHeadlines(QJsonDocument const &doc, QJsonObject const &request);
    Item deserializeHeadline(QJsonObject const &item, bool haveContent);
    void nextBatch(QJsonObject const &request, int skip, RequestCallback const &callback);

    void getFeeds();
    void gotFeeds(QJsonDocument const &doc, QJsonObject const &request);


    void syncReadStatus(qint64 feedId, qint64 unreadCount);
    void gotStatusUpdate(qint64 feedId, qint64 unreadCount, QJsonDocument const &doc, QJsonObject const &request);

    void gotAddFeedStatus(QJsonDocument const &replyDoc, QJsonObject const &requestData);

    void finishUpdate();
};

#endif // TTRSSFEEDSOURCE_H
