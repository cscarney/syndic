/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "saveapi.h"
#include <QHttpServerResponse>
#include <QObject>
#include <QPointer>
#include <memory>
#include <optional>

namespace FeedCore
{
class Context;
class Storage;
}
class PairingController;
class QHttpServer;
class QHttpServerRequest;

/**
 * Serves the "Save to Syndic" API (see doc/save-api.md) over HTTP on the
 * loopback interface, for the browser extension and other local clients.
 *
 * HTTP on 127.0.0.1 is used because it is reachable from (and to) Flatpak
 * sandboxes, unlike native messaging or D-Bus.
 */
class SaveServer : public QObject
{
    Q_OBJECT
public:
    SaveServer(FeedCore::Context *context, FeedCore::Storage *storage, PairingController *pairing, QObject *parent = nullptr);
    ~SaveServer();

    /**
     * Start listening on /port/, stopping any existing listener first.
     */
    bool listen(quint16 port);
    void close();

private:
    SaveApi *m_api;
    QPointer<PairingController> m_pairing;
    std::unique_ptr<QHttpServer> m_server;
    quint16 m_port{0};

    void setupRoutes();

    /**
     * Route an authenticated POST at /path/ to a SaveApi method.
     */
    void routeApiMethod(const QString &path, QFuture<SaveApi::Response> (SaveApi::*method)(const QJsonObject &));

    /**
     * Returns an error response if the request must be rejected.
     */
    std::optional<QHttpServerResponse> rejectRequest(const QHttpServerRequest &request, bool requireAuth) const;
};
