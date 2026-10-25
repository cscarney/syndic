/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "pairingcontroller.h"
#include "future.h"
#include "settings.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QTimer>
#include <QUuid>

static constexpr const int kRequestLifetimeSecs = 300;
static constexpr const int kTokenBytes = 32;
static constexpr const int kMaxClientNameLength = 80;

PairingController::PairingController(Settings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

QString PairingController::hashToken(const QByteArray &token)
{
    return QString::fromLatin1(QCryptographicHash::hash(token, QCryptographicHash::Sha256).toHex());
}

PairingController::Request PairingController::beginPairing(const QString &clientName)
{
    if (!m_request.id.isEmpty() && m_request.status == Pending) {
        // the previous request is superseded
        m_request.status = Denied;
        emit decided(m_request.id);
    }

    QString name = clientName.simplified().left(kMaxClientNameLength);
    if (name.isEmpty()) {
        name = tr("Unknown application");
    }
    m_request = PendingRequest{
        QUuid::createUuid().toString(QUuid::WithoutBraces),
        name,
        QStringLiteral("%1").arg(QRandomGenerator::system()->bounded(10000), 4, 10, QLatin1Char('0')),
        QDateTime::currentDateTimeUtc().addSecs(kRequestLifetimeSecs),
        Pending,
        {},
    };
    emit pendingChanged();
    emit pairingRequested();
    return {m_request.id, m_request.code};
}

QFuture<PairingController::Decision> PairingController::waitForDecision(const QString &requestId, int timeout)
{
    auto takeDecision = [this, requestId]() -> std::optional<Decision> {
        if (requestId.isEmpty() || requestId != m_request.id) {
            return Decision{Unknown, {}};
        }
        if (m_request.status == Pending && QDateTime::currentDateTimeUtc() > m_request.expires) {
            m_request.status = Denied;
            emit pendingChanged();
        }
        if (m_request.status == Pending) {
            return std::nullopt;
        }
        Decision decision{m_request.status, m_request.token};
        // hand out the token once only
        m_request = {};
        return decision;
    };

    if (auto decision = takeDecision()) {
        return QtFuture::makeReadyValueFuture(*decision);
    }

    auto promise = FeedCore::Future::makeSharedPromise<Decision>();
    auto *guard = new QObject(this);
    auto settle = [guard, promise, takeDecision](bool timedOut) {
        if (promise->future().isFinished()) {
            return;
        }
        std::optional<Decision> decision = takeDecision();
        if (!decision && !timedOut) {
            return;
        }
        guard->deleteLater();
        FeedCore::Future::fulfill(promise, decision.value_or(Decision{Pending, {}}));
    };
    QObject::connect(this, &PairingController::decided, guard, [settle] {
        settle(false);
    });
    QTimer::singleShot(timeout, guard, [settle] {
        settle(true);
    });
    return promise->future();
}

void PairingController::respond(bool approve)
{
    if (m_request.id.isEmpty() || m_request.status != Pending) {
        return;
    }
    if (approve) {
        QByteArray tokenBytes(kTokenBytes, Qt::Uninitialized);
        QRandomGenerator::system()->fillRange(reinterpret_cast<quint32 *>(tokenBytes.data()), kTokenBytes / sizeof(quint32));
        const QByteArray token = tokenBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        QStringList clients = m_settings->pairedClients();
        clients << hashToken(token) + QLatin1Char(' ') + m_request.clientName;
        m_settings->setPairedClients(clients);
        m_settings->save();
        m_request.token = QString::fromLatin1(token);
        m_request.status = Approved;
        emit pairedClientsChanged();
    } else {
        m_request.status = Denied;
    }
    emit pendingChanged();
    emit decided(m_request.id);
}

bool PairingController::isAuthorized(const QByteArray &token) const
{
    if (token.isEmpty()) {
        return false;
    }
    const QString hash = hashToken(token);
    const QStringList clients = m_settings->pairedClients();
    for (const QString &client : clients) {
        if (client.section(QLatin1Char(' '), 0, 0) == hash) {
            return true;
        }
    }
    return false;
}

void PairingController::forgetPairedClients()
{
    m_settings->setPairedClients({});
    m_settings->save();
    emit pairedClientsChanged();
}

bool PairingController::pending() const
{
    return !m_request.id.isEmpty() && m_request.status == Pending;
}

QString PairingController::pendingClientName() const
{
    return pending() ? m_request.clientName : QString();
}

QString PairingController::pendingCode() const
{
    return pending() ? m_request.code : QString();
}

int PairingController::pairedClientCount() const
{
    return int(m_settings->pairedClients().size());
}
