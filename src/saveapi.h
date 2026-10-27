/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QFuture>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

namespace FeedCore
{
class Context;
class PageSaver;
}

/**
 * JSON front-end for PageSaver ("Save to Syndic").
 *
 * This class implements the request and response bodies described in
 * doc/save-api.md, and the request checks that don't depend on the HTTP
 * server, so that they can be tested without it. SaveServer authenticates
 * the client before calling resolve() or save().
 */
class SaveApi : public QObject
{
    Q_OBJECT
public:
    static constexpr const int kApiVersion = 1;

    struct Response {
        int status{200}; /** < HTTP-style status code */
        QJsonObject body;
    };

    SaveApi(FeedCore::Context *context, FeedCore::PageSaver *saver, QObject *parent = nullptr);

    /**
     * /paired/ is whether the request carried a valid token.
     */
    static Response ping(bool paired);
    QFuture<Response> resolve(const QJsonObject &body);
    QFuture<Response> save(const QJsonObject &body);

    static Response error(int status, const QString &code);

    /**
     * Whether a request carrying this Origin header may use the API. Browser extensions
     * and non-browser clients (no Origin) are allowed; web pages are not.
     */
    static bool isAllowedOrigin(const QByteArray &origin);

    /**
     * Whether a Host header is acceptable for a server listening on the loopback interface
     * at /port/. Guards against DNS rebinding.
     */
    static bool isAllowedHost(const QByteArray &host, quint16 port);

private:
    FeedCore::Context *m_context;
    FeedCore::PageSaver *m_saver;
    QJsonArray feedList() const;
};
