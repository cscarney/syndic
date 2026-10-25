/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QDateTime>
#include <QFuture>
#include <QObject>

class Settings;

/**
 * Manages the access tokens of external clients (e.g. the browser extension)
 * that may save pages to Syndic.
 *
 * A client requests pairing with beginPairing(); the request is shown to the
 * user, who approves or denies it with respond(). Approved clients receive a
 * token, which is checked with isAuthorized(). Only hashes of the tokens are
 * stored.
 */
class PairingController : public QObject
{
    Q_OBJECT

    /**
     * Whether a pairing request is waiting for the user's decision.
     */
    Q_PROPERTY(bool pending READ pending NOTIFY pendingChanged)

    /**
     * The name the pending client gave for itself.
     */
    Q_PROPERTY(QString pendingClientName READ pendingClientName NOTIFY pendingChanged)

    /**
     * A short code that is displayed by both the client and Syndic, so the user can
     * confirm that they are approving the right request.
     */
    Q_PROPERTY(QString pendingCode READ pendingCode NOTIFY pendingChanged)

    Q_PROPERTY(int pairedClientCount READ pairedClientCount NOTIFY pairedClientsChanged)

public:
    explicit PairingController(Settings *settings, QObject *parent = nullptr);

    struct Request {
        QString id;
        QString code;
    };

    enum Status { Pending, Approved, Denied, Unknown };

    struct Decision {
        Status status{Unknown};
        QString token;
    };

    /**
     * Start a pairing request, replacing any request that is already pending.
     */
    Request beginPairing(const QString &clientName);

    /**
     * Wait up to /timeout/ msecs for the user to decide on a pairing request.
     *
     * The token of an approved request is only handed out once.
     */
    QFuture<Decision> waitForDecision(const QString &requestId, int timeout);

    bool isAuthorized(const QByteArray &token) const;

    Q_INVOKABLE void respond(bool approve);
    Q_INVOKABLE void forgetPairedClients();

    bool pending() const;
    QString pendingClientName() const;
    QString pendingCode() const;
    int pairedClientCount() const;

signals:
    void pendingChanged();
    void pairedClientsChanged();

    /**
     * Emitted when a new pairing request needs the user's attention.
     */
    void pairingRequested();

    /**
     * Emitted when the request /requestId/ has been decided.
     */
    void decided(const QString &requestId);

private:
    struct PendingRequest {
        QString id;
        QString clientName;
        QString code;
        QDateTime expires;
        Status status{Pending};
        QString token;
    };

    Settings *m_settings;
    PendingRequest m_request;

    static QString hashToken(const QByteArray &token);
};
