/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
#define FEEDCORE_CACHEDNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

namespace FeedCore {
class NetworkAccessManager : public QNetworkAccessManager
{
public:
    static NetworkAccessManager *instance();

    explicit NetworkAccessManager(QObject *parent=nullptr);
    explicit NetworkAccessManager(QAbstractNetworkCache *cache, QObject *parent=nullptr);
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData) override;
};
}

#endif // FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
