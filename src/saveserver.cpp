/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "saveserver.h"
#include "pairingcontroller.h"
#include "saveapi.h"
#include <QDebug>
#include <QHttpHeaders>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>

using namespace FeedCore;
using StatusCode = QHttpServerResponse::StatusCode;

static constexpr const int kPairingWaitMsecs = 25000;
static constexpr const qsizetype kMaxBodySize = 64 * 1024 * 1024;

namespace
{
QHttpServerResponse jsonResponse(const QJsonObject &body, int status, const QByteArray &origin)
{
    QHttpServerResponse response(body, static_cast<StatusCode>(status));
    QHttpHeaders headers = response.headers();
    if (!origin.isEmpty() && SaveApi::isAllowedOrigin(origin)) {
        headers.append("Access-Control-Allow-Origin", origin);
        headers.append("Vary", "Origin");
    }
    headers.append("Cache-Control", "no-store");
    response.setHeaders(std::move(headers));
    return response;
}

QHttpServerResponse jsonResponse(const QJsonObject &body, int status, const QHttpServerRequest &request)
{
    return jsonResponse(body, status, request.value("Origin"));
}

QHttpServerResponse jsonResponse(const SaveApi::Response &response, const QHttpServerRequest &request)
{
    return jsonResponse(response.body, response.status, request);
}

QHttpServerResponse errorResponse(int status, const QString &code, const QHttpServerRequest &request)
{
    return jsonResponse(SaveApi::error(status, code), request);
}

QByteArray bearerToken(const QHttpServerRequest &request)
{
    const QByteArray header = request.value("Authorization");
    static const QByteArray kPrefix = "Bearer ";
    if (!header.startsWith(kPrefix)) {
        return {};
    }
    return header.mid(kPrefix.size()).trimmed();
}

std::optional<QJsonObject> parseBody(const QHttpServerRequest &request)
{
    const QByteArray body = request.body();
    if (body.isEmpty()) {
        return QJsonObject();
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(body, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return std::nullopt;
    }
    return document.object();
}
}

SaveServer::SaveServer(Context *context, PairingController *pairing, QObject *parent)
    : QObject(parent)
    , m_api(new SaveApi(context, this))
    , m_pairing(pairing)
{
}

SaveServer::~SaveServer() = default;

bool SaveServer::listen(quint16 port)
{
    close();
    auto server = std::make_unique<QHttpServer>();
    auto *tcpServer = new QTcpServer(server.get());
    if (!tcpServer->listen(QHostAddress::LocalHost, port)) {
        qWarning() << "Save to Syndic: couldn't listen on port" << port << tcpServer->errorString();
        return false;
    }
    if (!server->bind(tcpServer)) {
        qWarning() << "Save to Syndic: couldn't bind HTTP server";
        return false;
    }
    m_server = std::move(server);
    m_port = port;
    setupRoutes();
    return true;
}

void SaveServer::close()
{
    m_server.reset();
    m_port = 0;
}

std::optional<QHttpServerResponse> SaveServer::rejectRequest(const QHttpServerRequest &request, bool requireAuth) const
{
    // Only accept requests addressed to the loopback interface by name, to defeat DNS
    // rebinding, and only from extensions or non-browser clients, not from web pages.
    if (!SaveApi::isAllowedHost(request.value("Host"), m_port)) {
        return errorResponse(403, QStringLiteral("forbidden-host"), request);
    }
    if (!SaveApi::isAllowedOrigin(request.value("Origin"))) {
        return errorResponse(403, QStringLiteral("forbidden-origin"), request);
    }
    if (request.body().size() > kMaxBodySize) {
        return errorResponse(413, QStringLiteral("too-large"), request);
    }
    if (requireAuth && (m_pairing.isNull() || !m_pairing->isAuthorized(bearerToken(request)))) {
        return errorResponse(401, QStringLiteral("unauthorized"), request);
    }
    return std::nullopt;
}

void SaveServer::setupRoutes()
{
    using Method = QHttpServerRequest::Method;

    m_server->route("/api/v1/ping", Method::Get, this, [this](const QHttpServerRequest &request) {
        if (auto rejection = rejectRequest(request, false)) {
            return std::move(*rejection);
        }
        return jsonResponse(SaveApi::ping(m_pairing && m_pairing->isAuthorized(bearerToken(request))), request);
    });

    m_server->route("/api/v1/pair", Method::Post, this, [this](const QHttpServerRequest &request) {
        if (auto rejection = rejectRequest(request, false)) {
            return std::move(*rejection);
        }
        const auto body = parseBody(request);
        if (!body) {
            return errorResponse(400, QStringLiteral("invalid-json"), request);
        }
        const auto pairing = m_pairing->beginPairing(body->value(QStringLiteral("clientName")).toString());
        return jsonResponse(QJsonObject{{QStringLiteral("requestId"), pairing.id}, {QStringLiteral("code"), pairing.code}}, 200, request);
    });

    m_server->route("/api/v1/pair/wait", Method::Post, this, [this](const QHttpServerRequest &request) -> QFuture<QHttpServerResponse> {
        if (auto rejection = rejectRequest(request, false)) {
            return QtFuture::makeReadyValueFuture(std::move(*rejection));
        }
        const auto body = parseBody(request);
        if (!body) {
            return QtFuture::makeReadyValueFuture(errorResponse(400, QStringLiteral("invalid-json"), request));
        }
        auto decision = m_pairing->waitForDecision(body->value(QStringLiteral("requestId")).toString(), kPairingWaitMsecs);
        return decision.then(this, [origin = request.value("Origin")](const PairingController::Decision &decision) {
            QJsonObject body;
            int status = 200;
            switch (decision.status) {
            case PairingController::Approved:
                body = {{QStringLiteral("status"), QStringLiteral("approved")}, {QStringLiteral("token"), decision.token}};
                break;
            case PairingController::Pending:
                body = {{QStringLiteral("status"), QStringLiteral("pending")}};
                break;
            case PairingController::Denied:
                body = {{QStringLiteral("status"), QStringLiteral("denied")}};
                status = 403;
                break;
            case PairingController::Unknown:
                body = {{QStringLiteral("status"), QStringLiteral("unknown")}};
                status = 404;
                break;
            }
            return jsonResponse(body, status, origin);
        });
    });

    routeApiMethod(QStringLiteral("/api/v1/resolve"), &SaveApi::resolve);
    routeApiMethod(QStringLiteral("/api/v1/save"), &SaveApi::save);

    // CORS preflight, for extensions that aren't exempt from CORS
    m_server->route("/api/v1/<arg>", Method::Options, this, [this](const QString &, const QHttpServerRequest &request) {
        const QByteArray origin = request.value("Origin");
        if (!SaveApi::isAllowedHost(request.value("Host"), m_port) || origin.isEmpty() || !SaveApi::isAllowedOrigin(origin)) {
            return QHttpServerResponse(StatusCode::Forbidden);
        }
        QHttpServerResponse response(StatusCode::NoContent);
        QHttpHeaders headers;
        headers.append("Access-Control-Allow-Origin", origin);
        headers.append("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        headers.append("Access-Control-Allow-Headers", "Authorization, Content-Type");
        headers.append("Access-Control-Max-Age", "600");
        response.setHeaders(std::move(headers));
        return response;
    });

    m_server->setMissingHandler(this, [](const QHttpServerRequest &request, QHttpServerResponder &responder) {
        responder.sendResponse(errorResponse(404, QStringLiteral("not-found"), request));
    });
}

void SaveServer::routeApiMethod(const QString &path, QFuture<SaveApi::Response> (SaveApi::*method)(const QJsonObject &))
{
    m_server->route(path, QHttpServerRequest::Method::Post, this, [this, method](const QHttpServerRequest &request) -> QFuture<QHttpServerResponse> {
        if (auto rejection = rejectRequest(request, true)) {
            return QtFuture::makeReadyValueFuture(std::move(*rejection));
        }
        const auto body = parseBody(request);
        if (!body) {
            return QtFuture::makeReadyValueFuture(errorResponse(400, QStringLiteral("invalid-json"), request));
        }
        return (m_api->*method)(*body).then(this, [origin = request.value("Origin")](const SaveApi::Response &result) {
            return jsonResponse(result.body, result.status, origin);
        });
    });
}
