#include "ttrssfeedsource.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define MEMBER_CALLBACK(funcname) [this](auto doc,auto request){ funcname (doc, request); }

static QJsonDocument docFromReply(QNetworkReply *reply)
{
    auto doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    return doc;
}

TTRSSFeedSource::TTRSSFeedSource(QObject *parent)
    : FeedSource(parent)
{
    m_settings.beginGroup("TTRSSFeedSource");
    m_host = m_settings.value("host").toString();
    m_user = m_settings.value("user").toString();
    m_password = m_settings.value("password").toString();
    m_lastId = m_settings.value("lastId", 0).toLongLong();
}

void TTRSSFeedSource::beginLogin()
{
    if (m_loginInProgress){
        qDebug("Waiting on login");
    } else {
        m_loginInProgress = true;
        qDebug("begin login");
        m_pendingRequests++;
        performRequest({
            {"op", "login"},
            {"user", m_user},
            {"password", m_password}
        }, MEMBER_CALLBACK(gotLogin));
    }
}

void TTRSSFeedSource::gotLogin(QJsonDocument const &doc, QJsonObject const & /*request*/)
{
    m_sessionId = doc["content"].toObject()["session_id"].toString();
    qDebug("end login");
    m_pendingRequests--;
    m_loginInProgress = false;
    if (!m_sessionId.isNull()) {
        loginSuccess();
    } else {
        qDebug("login failed");
        updateError("Login Failed");
    }

    QObject::disconnect(this, &TTRSSFeedSource::loginSuccess, nullptr, nullptr);
}

void TTRSSFeedSource::beginUpdate()
{
    if (m_pendingRequests > 0) {
        qDebug("already updating");
        return;
    }
    m_fullUpdate = (m_lastId == 0);

    qDebug("getting new items since %lld", m_lastId);
    performAuthenticatedRequest({
        {"op", "getHeadlines"},
        {"feed_id", -4},
        {"view_mode", "all_articles"},
        {"show_content", true},
        {"since_id", m_lastId}
    }, MEMBER_CALLBACK(gotHeadlines));

    performAuthenticatedRequest({
        {"op", "getFeeds"},
        {"cat_id", -3}
    }, MEMBER_CALLBACK(gotFeeds));
}

void TTRSSFeedSource::setFlag(qint64 id, ItemFlag flag, bool state)
{
    switch (flag) {
        case ItemFlag::UNREAD:
            performAuthenticatedRequest({
                {"op", "updateArticle"},
                {"article_ids", QString("%1").arg(id)},
                {"mode", state?1:0},
                {"field", 2}
            }, nullptr);
        break;

        case ItemFlag::STARRED:
            performAuthenticatedRequest({
                {"op", "updateArticle"},
                {"article_ids", QString("%1").arg(id)},
                {"mode", state?1:0},
                {"field", 0}
            }, nullptr);
        break;

        default:
            break;
    }
}

inline void TTRSSFeedSource::dispatchResult(RequestCallback const &callback, QJsonDocument const &doc, QJsonObject const &request)
{
    if (doc["status"].toInt() != 0) {
        auto errorString = doc["error"].toString();
        updateError(QString("Got error from server: %1").arg(errorString));
        qDebug("error: %s", errorString.toStdString().c_str());
    } else {
        if (callback) callback(doc, request);
    }
}

void TTRSSFeedSource::performAuthenticatedRequest(QJsonObject const &requestData, RequestCallback const &callback)
{
    if (m_sessionId.isNull()) {
        qDebug("not logged in");
        QObject::connect(this, &TTRSSFeedSource::loginSuccess, [=]{
            performAuthenticatedRequest(requestData, callback);
        });
        beginLogin();
    } else {
        m_pendingRequests++;
        auto authenticatedData = requestData;
        authenticatedData.insert("sid", m_sessionId);
        performRequest(authenticatedData, [this, callback](auto reply, auto requestData) {
            gotAuthenticatedReply(reply, requestData, callback);
        });
    }
}

void TTRSSFeedSource::gotAuthenticatedReply(QJsonDocument const &replyDoc, QJsonObject const &requestData, TTRSSFeedSource::RequestCallback const &callback)
{
    auto content = replyDoc["content"];
    auto error = content.toObject()["error"].toString();
    if (error == QString("NOT_LOGGED_IN")) {
        qDebug("session expired");
        m_sessionId = nullptr;
        performAuthenticatedRequest(requestData, callback);
    } else {
        dispatchResult(callback, replyDoc, requestData);
    }
    m_pendingRequests--;
    if (m_pendingRequests <= 0) {
        finishUpdate();
    }
}

QNetworkReply * TTRSSFeedSource::performRequest(QJsonObject const &requestData)
{
    QNetworkRequest request(m_host);
    request.setHeader(QNetworkRequest::UserAgentHeader, "curl/4.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    auto requestDoc = QJsonDocument(requestData).toJson();
    return m_nam.post(request, requestDoc);
}

void TTRSSFeedSource::performRequest(QJsonObject const &requestData, RequestCallback const &callback)
{
    auto *reply = performRequest(requestData);
    QObject::connect(reply, &QNetworkReply::finished, this, [=]{
        auto doc = docFromReply(reply);
        dispatchResult(callback, doc, requestData);
    });
}

void TTRSSFeedSource::processContent(QJsonArray const &entries, bool haveContent)
{
    for (auto i=entries.begin(); i!=entries.end(); ++i)
    {
        auto entry = i->toObject();
        auto timestamp = entry["updated"].toInt();
        auto time = QDateTime::fromSecsSinceEpoch(timestamp);
        auto id = entry["id"].toInt();
        if (m_lastId < id)
            m_lastId = id;

        foundContent({
            .id=id,
            .feedId=entry["feed_id"].toString().toInt(),
            .headline=entry["title"].toString(),
            .author=entry["author"].toString(),
            .date=time,
            .content=haveContent ? entry["content"].toString() : QString(),
            .url=entry["link"].toString(),
            .isUnread=entry["unread"].toBool(),
            .isStarred=entry["marked"].toBool()
         });
    }
}

void TTRSSFeedSource::nextBatch(QJsonObject const &request, int skip, RequestCallback const &callback)
{
    if (skip==0) return;
    auto nextBatchRequest = request;
    auto oldOffset = request["skip"].toInt();
    nextBatchRequest["skip"] = oldOffset + skip;
    performAuthenticatedRequest(nextBatchRequest, callback);
}

void TTRSSFeedSource::gotHeadlines(QJsonDocument const &doc, QJsonObject const &request)
{
    auto content = doc["content"];
    auto entries = content.toArray();
    qDebug("Got %d new items", entries.size());

    processContent(entries, request["show_content"].toBool());
    nextBatch(request, entries.size(), MEMBER_CALLBACK(gotHeadlines));
}

void TTRSSFeedSource::gotFeeds(QJsonDocument const &doc, QJsonObject const & /*request*/)
{
    auto content = doc["content"];
    auto entries = content.toArray();
    for (auto i=entries.begin(); i!=entries.end(); ++i)
    {
        auto entry = i->toObject();
        auto feedId = entry["id"].toInt();
        foundFeed({
            .id=feedId,
            .name=entry["title"].toString()
        });
        if (!m_fullUpdate){
            syncReadStatus(feedId, entry["unread"].toInt());
        }
    }
}

void TTRSSFeedSource::syncReadStatus(qint64 feedId, qint64 unreadCount)
{
    if (unreadCount>0) {
        /* TODO we don't need to suck down all of the headlines.
         * We can get smaller batches (w/ limit param) and stop
         * when we've seen $unread items */
        performAuthenticatedRequest({
            {"op", "getHeadlines"},
            {"feed_id", feedId},
            {"view_mode", "all_articles"},
            {"show_content", false}
        }, MEMBER_CALLBACK(gotHeadlines));
    } else {
        feedRead(feedId);
    }
}

void TTRSSFeedSource::finishUpdate()
{
    qDebug("update complete");
    m_pendingRequests = 0;
    m_settings.setValue("lastId", m_lastId);
}






