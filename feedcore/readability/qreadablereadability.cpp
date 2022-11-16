/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qreadablereadability.h"
#include "networkaccessmanager.h"
#include <QDebug>
#include <QNetworkReply>
#include <QString>

#include <qreadable/readable.h>

using namespace FeedCore;

QReadableReadability::QReadableReadability()
    : m_readable{new QReadable::Readable(this)}
{
}

void QReadableReadability::fetch(const QString &url)
{
    auto *nam = NetworkAccessManager::instance();
    QNetworkRequest req(url);
    auto *reply = nam->get(req);
    emit startedFetching(url);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, url, reply] {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QString rawHtml(data);
            QString readableHtml = m_readable->parse(rawHtml, reply->url());
            emit finishedFetching(url, readableHtml);
        } else {
            emit errorFetching(url);
        }
    });
}
