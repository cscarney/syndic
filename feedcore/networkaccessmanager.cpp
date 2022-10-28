/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "networkaccessmanager.h"
#include "sharedcache.h"
#include <QNetworkReply>
#include <QStack>
using namespace FeedCore;

/* This is an (incomlete) proxy for QNetworkReply that gives us something
 * to return to the caller when we queue a request for later.
 *
 * The network access manager calls start() with the actual reply when
 * the request gets dequeued.
 */
class NetworkAccessManager::DeferredNetworkReply : public QNetworkReply
{
    QNetworkReply *m_reply{nullptr};
    NetworkAccessManager *m_nam;

public:
    explicit DeferredNetworkReply(NetworkAccessManager *parent);
    ~DeferredNetworkReply();
    void start(QNetworkReply *reply);

    qint64 readData(char *data, qint64 max) override;
    void abort() override;
    bool isSequential() const override;
    void forwardSignals();
    void forwardAttribute(QNetworkRequest::Attribute attr);
    void forwardHeaders();
};

struct NetworkAccessManager::WaitingRequest {
    QNetworkAccessManager::Operation op{QNetworkAccessManager::GetOperation};
    QNetworkRequest req{};
    QIODevice *outgoingData{nullptr};
    DeferredNetworkReply *repl{nullptr};
};

struct NetworkAccessManager::PrivData {
    NetworkAccessManager *parent;
    int connectionCount{0};
    QStack<WaitingRequest> waitingRequests;

    explicit PrivData(NetworkAccessManager *parent)
        : parent(parent)
    {
    }
    void startWaiting();
    QNetworkReply *makeRealReply(const WaitingRequest &wr);
    void removeWaiting(DeferredNetworkReply *reply);
};

NetworkAccessManager::DeferredNetworkReply::DeferredNetworkReply(NetworkAccessManager *parent)
    : QNetworkReply(parent)
    , m_nam(parent)
{
    setOpenMode(ReadOnly | Unbuffered);
}

NetworkAccessManager::DeferredNetworkReply::~DeferredNetworkReply()
{
    if (m_reply != nullptr) {
        m_reply->deleteLater();
    }
}

void NetworkAccessManager::DeferredNetworkReply::start(QNetworkReply *reply)
{
    m_reply = reply;
    setOperation(reply->operation());
    setRequest(reply->request());
    setUrl(reply->url());
    forwardSignals();

    QObject::connect(reply, &QNetworkReply::redirected, this, &DeferredNetworkReply::setUrl);

    // handle auto-deleted replies
    QObject::connect(reply, &QObject::destroyed, this, [this] {
        m_reply = nullptr;
        deleteLater();
    });
}

qint64 NetworkAccessManager::DeferredNetworkReply::readData(char *data, qint64 max)
{
    if (m_reply == nullptr || !m_reply->isOpen()) {
        return 0;
    }
    return m_reply->read(data, max);
}

void NetworkAccessManager::DeferredNetworkReply::abort()
{
    if (m_reply == nullptr) {
        setError(QNetworkReply::OperationCanceledError, "");
        emit finished();
        m_nam->d->removeWaiting(this);
        return;
    }
    m_reply->abort();
}

bool NetworkAccessManager::DeferredNetworkReply::isSequential() const
{
    if (m_reply == nullptr) {
        return true;
    }
    return m_reply->isSequential();
}

void NetworkAccessManager::DeferredNetworkReply::forwardSignals()
{
    QObject::connect(m_reply, &QNetworkReply::finished, this, &QNetworkReply::finished);
    QObject::connect(m_reply, &QIODevice::readyRead, this, &QIODevice::readyRead);
    QObject::connect(m_reply, &QNetworkReply::errorOccurred, this, [this](auto code) {
        setError(code, m_reply->errorString());
    });
    QObject::connect(m_reply, &QNetworkReply::metaDataChanged, this, &DeferredNetworkReply::forwardHeaders);
}

void NetworkAccessManager::DeferredNetworkReply::forwardAttribute(QNetworkRequest::Attribute attr)
{
    setAttribute(attr, m_reply->attribute(attr));
}

void NetworkAccessManager::DeferredNetworkReply::forwardHeaders()
{
    // this is *slow* but I don't see a way to get only the ones that changed
    const QList<QByteArray> headers = m_reply->rawHeaderList();
    for (const QByteArray &headerName : headers) {
        setRawHeader(headerName, m_reply->rawHeader(headerName));
    }

    forwardAttribute(QNetworkRequest::CacheLoadControlAttribute);
    forwardAttribute(QNetworkRequest::CacheSaveControlAttribute);
    forwardAttribute(QNetworkRequest::HttpStatusCodeAttribute);
    forwardAttribute(QNetworkRequest::RedirectionTargetAttribute);
    forwardAttribute(QNetworkRequest::SourceIsFromCacheAttribute);
}

NetworkAccessManager *NetworkAccessManager::instance()
{
    static auto *singleton = new NetworkAccessManager(new SharedCache);
    return singleton;
}

FeedCore::NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : NetworkAccessManager(nullptr, parent)
{
}

NetworkAccessManager::NetworkAccessManager(QAbstractNetworkCache *cache, QObject *parent)
    : QNetworkAccessManager(parent)
    , d{std::make_unique<PrivData>(this)}
{
    setCache(cache);
}

NetworkAccessManager::~NetworkAccessManager() = default;

static void setDefaultAttribute(QNetworkRequest &req, QNetworkRequest::Attribute attrName, QVariant attrVal)
{
    if (!req.attribute(attrName).isValid()) {
        req.setAttribute(attrName, attrVal);
    }
}

QNetworkReply *FeedCore::NetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QNetworkRequest newRequest(request);
    newRequest.setHeader(QNetworkRequest::UserAgentHeader, "syndic/1.0");
    setDefaultAttribute(newRequest, QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    newRequest.setTransferTimeout();

    // We don't want to keep connections open since we make one-shot downloads
    // from a lot of different servers in rapid succession.
    newRequest.setRawHeader("Connection", "close");

    if (d->connectionCount < kMaxSimultaneousLoads) {
        return d->makeRealReply({op, newRequest, outgoingData});
    }
    auto *proxyReply = new DeferredNetworkReply(this);
    d->waitingRequests.push({op, newRequest, outgoingData, proxyReply});
    return proxyReply;
}

void NetworkAccessManager::onFinished()
{
    d->connectionCount--;
    d->startWaiting();
    if (d->connectionCount == 0) {
        clearConnectionCache();
    }
}

void NetworkAccessManager::PrivData::startWaiting()
{
    while (connectionCount < NetworkAccessManager::kMaxSimultaneousLoads && !waitingRequests.isEmpty()) {
        makeRealReply(waitingRequests.pop());
    }
}

QNetworkReply *NetworkAccessManager::PrivData::makeRealReply(const WaitingRequest &wr)
{
    QNetworkReply *realReply = parent->QNetworkAccessManager::createRequest(wr.op, wr.req, wr.outgoingData);
    QObject::connect(realReply, &QNetworkReply::finished, parent, &NetworkAccessManager::onFinished);
    if (wr.repl != nullptr) {
        wr.repl->start(realReply);
    }
    connectionCount++;
    return realReply;
}

void NetworkAccessManager::PrivData::removeWaiting(DeferredNetworkReply *reply)
{
    QVector<WaitingRequest>::iterator it = std::find_if(waitingRequests.begin(), waitingRequests.end(), [reply](const WaitingRequest &wr) {
        return wr.repl == reply;
    });
    if (it != waitingRequests.end()) {
        waitingRequests.erase(it);
        if (parent->autoDeleteReplies() || reply->request().attribute(QNetworkRequest::AutoDeleteReplyOnFinishAttribute).toBool()) {
            reply->deleteLater();
        }
    }
}
