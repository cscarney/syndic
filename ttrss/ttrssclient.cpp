/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ttrss/ttrssclient.h"
#include "networkaccessmanager.h"
#include <QJsonObject>
#include <QNetworkReply>
#include <utility>
using namespace FeedCore;
using namespace TTRSS;

namespace
{

class ApiCallImpl : public Client::ApiCall
{
    QJsonObject m_body;

public:
    ApiCallImpl(QJsonObject body, QObject *parent)
        : Client::ApiCall(parent)
        , m_body(std::move(body))
    {
    }

    void performRequest(const QUrl &endpoint)
    {
        QNetworkRequest req(endpoint);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        req.setAttribute(QNetworkRequest::AutoDeleteReplyOnFinishAttribute, true);
        QByteArray doc = QJsonDocument(m_body).toJson(QJsonDocument::Compact);
        auto *reply = NetworkAccessManager::instance()->post(req, doc);
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, doc] {
            handleReply(reply);
            deleteLater();
        });
    }

    void cancel(const QString &errorMessage)
    {
        emit error(errorMessage);
        deleteLater();
    }

    void performRequest(const QUrl &endpoint, const QString &sessionId)
    {
        m_body.insert("sid", sessionId);
        performRequest(endpoint);
    }

private:
    void handleReply(QNetworkReply *reply)
    {
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.object()["status"].toInt() != 0) {
            emit error(doc["error"].toString());
            return;
        }
        emit success(doc);
    }
};

}

TTRSS::Client::ApiCall *TTRSS::Client::apiCall(const QJsonObject &body)
{
    auto *apiCall = new ApiCallImpl(body, this);
    apiCall->performRequest(m_apiEndpoint);
    return apiCall;
}

TTRSS::Client::ApiCall *TTRSS::Client::authenticatedApiCall(const QJsonObject &body)
{
    auto *apiCall = new ApiCallImpl(body, this);
    if (m_sessionId.isNull()) {
        QObject::connect(this, &TTRSS::Client::loginSuccess, apiCall, [this, body, apiCall] {
            apiCall->performRequest(m_apiEndpoint, m_sessionId);
        });
        QObject::connect(this, &TTRSS::Client::loginFailure, apiCall, [body, apiCall] {
            apiCall->cancel("login failure");
        });
        beginLogin();
    } else {
        apiCall->performRequest(m_apiEndpoint, m_sessionId);
    }
    return apiCall;
}

Client::ApiCall *Client::getFeeds()
{
    return authenticatedApiCall({{"op", "getFeeds"}, {"cat_id", -3}});
}

Client::ApiCall *Client::getArticles(int feedId, const QString &viewMode, int sinceId)
{
    return authenticatedApiCall({{"op", "getHeadlines"}, {"feed_id", feedId}, {"view_mode", viewMode}, {"since_id", sinceId}});
}

Client::ApiCall *Client::getArticle(int articleId)
{
    return authenticatedApiCall({{"op", "getArticle"}, {"article_id", articleId}});
}

Client::ApiCall *Client::setArticleFlag(int articleId, int field, bool value)
{
    return authenticatedApiCall({{"op", "updateArticle"}, {"article_ids", QStringLiteral("%1").arg(articleId)}, {"mode", value ? 1 : 0}, {"field", field}});
}

Client::Client() = default;

void Client::configure(const QUrl &apiEndpoint, const QString &userName, const QString &password)
{
    m_apiEndpoint = apiEndpoint;
    m_userName = userName;
    m_password = password;
}

void TTRSS::Client::beginLogin()
{
    if (m_pendingLogin) {
        return;
    }
    m_pendingLogin = true;
    m_sessionId.clear();
    ApiCall *call = apiCall({{"op", "login"}, {"user", m_userName}, {"password", m_password}});
    call->onSuccess(this, [this](auto doc) {
        m_sessionId = doc["content"].toObject()["session_id"].toString();
        m_pendingLogin = false;
        if (m_sessionId.isNull()) {
            qDebug() << "bad session id";
            emit loginFailure();
        } else {
            emit loginSuccess();
        }
    });
    call->onError(this, [this]() {
        qDebug() << "login api error";
        emit loginFailure();
    });
}
