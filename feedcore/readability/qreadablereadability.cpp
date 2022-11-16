/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qreadablereadability.h"
#include "networkaccessmanager.h"
#include "readabilityresult.h"
#include <QDebug>
#include <QNetworkReply>
#include <QString>

#include <qreadable/readable.h>

using namespace FeedCore;

class QReadableReadability::Result : public ReadabilityResult
{
    QNetworkReply *m_reply;
    QReadableReadability *m_readability;

public:
    Result(QReadableReadability *parent, QNetworkReply *reply)
        : ReadabilityResult(parent)
        , m_reply{reply}
        , m_readability{parent}
    {
        reply->setParent(this);
        QObject::connect(reply, &QNetworkReply::finished, this, &Result::onNetworkReplyFinished);
    }

    void onNetworkReplyFinished()
    {
        deleteLater();
        if (m_reply->error() != QNetworkReply::NoError) {
            emit error();
            return;
        }
        QByteArray data = m_reply->readAll();
        QString rawHtml(data);
        QString readableHtml = m_readability->parse(rawHtml, m_reply->url());
        emit finished(readableHtml);
    }
};

QReadableReadability::QReadableReadability()
    : m_readable{new QReadable::Readable(this)}
{
}

ReadabilityResult *QReadableReadability::fetch(const QUrl &url)
{
    auto *nam = NetworkAccessManager::instance();
    QNetworkRequest req(url);
    auto *reply = nam->get(req);
    return new Result(this, reply);
}

QString QReadableReadability::parse(const QString &rawHtml, const QUrl &url)
{
    return m_readable->parse(rawHtml, url);
}
