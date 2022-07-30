/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
#define FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
#include <QNetworkAccessManager>
#include <memory>

namespace FeedCore
{

class NetworkAccessManager : public QNetworkAccessManager
{
public:
    static NetworkAccessManager *instance();

    explicit NetworkAccessManager(QObject *parent = nullptr);
    explicit NetworkAccessManager(QAbstractNetworkCache *cache, QObject *parent = nullptr);
    ~NetworkAccessManager();
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData) override;

private:
    static constexpr const int kMaxSimultaneousLoads = 128;
    struct PrivData;
    class DeferredNetworkReply;
    struct WaitingRequest;
    std::unique_ptr<PrivData> d;
    void onFinished();
};
}

#endif // FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
