/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <QJsonDocument>
#include <QObject>
#include <QString>

namespace TTRSS
{

class Client : public QObject
{
    Q_OBJECT
public:
    Client();

    class ApiCall;

    enum SpecialFeedId { SPECIAL_FEED_STARRED = -1, SPECIAL_FEED_PUBLISHED = -2, SPECIAL_FEED_FRESH = -3, SPECIAL_FEED_ALL = -4 };

    void configure(const QUrl &apiEndpoint, const QString &userName, const QString &password);
    void beginLogin();

    /**
     * reply prototype:
     * {
     *      "seq": 0,
     *      "status": 0,
     *      "content": [
     *          {
     *              "feed_url":"http:\/\/....xml",
     *              "title":"Feed Title",
     *              "id": 12,
     *              "unread": 23,
     *              "has_icon":true,
     *              "cat_id": 4,
     *              "last_updated":1589479930, (seconds since epoch)
     *              "order_id": 0
     *          }
     *       ]
     * }
     */
    ApiCall *getFeeds();

    /**
     *  reply prototype:
     *  {
     *      "seq": 0,
     *      "status": 0,
     *      "content": [
     *          {
     *              "id": 123,
     *              "guid":"SHA1:16e9806ca2ce5367dde4544f6f0ae5656e4783b7",
     *              "unread": true,
     *              "marked": false,
     *              "published": false,
     *              "title": "Hello World",
     *              "link": "https:\/\/....html",
     *              "feed_id": "12",
     *              "tags": [""],
     *              "labels": [],
     *              "feed_title": "Some Feed",
     *              "comments_count": 0,
     *              "comments_link": "",
     *              "always_display_attachments": false,
     *              "author": "John Smith",
     *              "score": 0,
     *              "note": null,
     *              "lang": "en",
     *              "content": "Blah blah blah"  (empty string if show_content is false)
     *          },
     *          ...
     *       ]
     * }
     */
    ApiCall *getArticles(int feedId, const QString &viewMode = "all_articles", int sinceId = 0);

    ApiCall *getArticle(int articleId);

    ApiCall *setArticleFlag(int articleId, int field, bool value);

signals:
    void loginSuccess();
    void loginFailure();

private:
    QUrl m_apiEndpoint;
    QString m_userName;
    QString m_password;
    QString m_sessionId;
    bool m_pendingLogin{false};

    ApiCall *apiCall(const QJsonObject &body);
    ApiCall *authenticatedApiCall(const QJsonObject &body);
};

class Client::ApiCall : public QObject
{
    Q_OBJECT
public:
    template<typename Slot>
    ApiCall *onSuccess(QObject *context, Slot slot)
    {
        QObject::connect(this, &ApiCall::success, context, slot);
        return this;
    }

    template<typename Slot>
    ApiCall *onError(QObject *context, Slot slot)
    {
        QObject::connect(this, &ApiCall::error, context, slot);
        return this;
    }

signals:
    void success(const QJsonDocument &response);
    void error(const QString &errorMessage);

protected:
    using QObject::QObject;
};

}
