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

/**
 * A specialized QNetworkAccessManager for bulk updates
 *
 * This class supports queuing connections when many resources
 * are requested simultaneously. Once the connection limit is
 * hit, it will begin returning DeferredNetworkReply instances,
 * which will proxy an underlying QNetworkReply when a connection
 * slot becomes available.
 *
 * \warning DeferredNetworkReply does not implement every feature
 * of the QNetworkReply API. Test before using features that
 * are not already being used elsewhere in the application.
 */
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
