/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "updatablefeed.h"
#include "networkaccessmanager.h"
#include <QNetworkReply>
#include <Syndication/DataRetriever>
#include <Syndication/Image>
#include <Syndication/Loader>
using namespace FeedCore;

namespace
{
class DataRetriever : public Syndication::DataRetriever
{
public:
    void retrieveData(const QUrl &url) final;
    int errorCode() const final;
    void abort() final;

private:
    QNetworkReply *m_reply{nullptr};
    void onRedirect(const QUrl &url);
    void onFinished();
};
}

class UpdatableFeed::UpdaterImpl : public Feed::Updater
{
public:
    UpdaterImpl(UpdatableFeed *feed, QObject *parent);
    void run() final;
    void abort() final;

private:
    Syndication::Loader *m_loader{nullptr};
    UpdatableFeed *m_updatableFeed{nullptr};
    bool m_sourceIsFeedDiscoveryResult{false};
    void loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status);
};

Feed::Updater *UpdatableFeed::updater()
{
    return m_updater;
}

FeedCore::UpdatableFeed::UpdatableFeed(QObject *parent)
    : Feed(parent)
    , m_updater{new UpdaterImpl(this, this)}
{
}

void UpdatableFeed::updateFromSource(const Syndication::FeedPtr &feed)
{
    if (name().isEmpty()) {
        setName(feed->title());
    }
    setLink(feed->link());
    setIcon(feed->icon()->url());
    const auto &items = feed->items();
    time_t expireTime = 0;
    if (expireAge() > 0) {
        expireTime = updater()->updateStartTime().toTime_t() - expireAge();
    }
    for (const auto &item : items) {
        const auto &dateUpdated = item->dateUpdated();
        if (dateUpdated == 0 || dateUpdated >= expireTime) {
            updateSourceArticle(item);
        }
    }
    if (expireTime > 0) {
        expire(QDateTime::fromTime_t(expireTime));
    }
}

UpdatableFeed::UpdaterImpl::UpdaterImpl(UpdatableFeed *feed, QObject *parent)
    : Updater(feed, parent)
    , m_updatableFeed{feed}
{
}

void UpdatableFeed::UpdaterImpl::run()
{
    if (!feed()->url().isValid()) {
        setError(tr("Invalid URL", "error message"));
        return;
    }
    m_loader = Syndication::Loader::create();
    QObject::connect(m_loader, &Syndication::Loader::loadingComplete, this, &UpdaterImpl::loadingComplete);
    m_loader->loadFrom(feed()->url(), new DataRetriever);
}

void UpdatableFeed::UpdaterImpl::abort()
{
    if (m_loader != nullptr) {
        m_loader->abort();
    }
}

void UpdatableFeed::UpdaterImpl::loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status)
{
    m_loader = nullptr;
    QString errorMessage;
    switch (status) {
    case Syndication::Success:
        m_updatableFeed->updateFromSource(content);
        finish();
        return;
    case Syndication::Aborted:
        aborted();
        return;
    case Syndication::Timeout:
        errorMessage = tr("Timeout", "error message");
        break;
    case Syndication::UnknownHost:
        errorMessage = tr("Unknown Host", "error message");
        break;
    case Syndication::FileNotFound:
        errorMessage = tr("File Not Found", "error message");
        break;
    case Syndication::OtherRetrieverError:
        errorMessage = tr("Retriever Error", "error message");
        break;
    case Syndication::InvalidXml:
        errorMessage = tr("Invalid XML", "error message");
        break;
    case Syndication::XmlNotAccepted:
        errorMessage = tr("XML Not Accepted", "error message");
        break;
    case Syndication::InvalidFormat:
        errorMessage = tr("Invalid Format", "error message");
        break;
    default:
        errorMessage = tr("Unknown Error", "error message");
    }

    // try the discovered url
    if ((!m_sourceIsFeedDiscoveryResult) && loader->discoveredFeedURL().isValid()) {
        qDebug() << "Discovered alternate source:" << loader->discoveredFeedURL();
        m_sourceIsFeedDiscoveryResult = true;
        m_updatableFeed->setUrl(loader->discoveredFeedURL());
        run();
    } else {
        qDebug() << "Error:" << errorMessage;
        setError(errorMessage);
    }
}

void DataRetriever::retrieveData(const QUrl &url)
{
    QNetworkRequest request(url);
    m_reply = NetworkAccessManager::instance()->get(request);
    QObject::connect(m_reply, &QNetworkReply::finished, this, &DataRetriever::onFinished);
}

int DataRetriever::errorCode() const
{
    return 0;
}

void DataRetriever::abort()
{
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
}

void DataRetriever::onFinished()
{
    m_reply->deleteLater();
    if (m_reply->error() == QNetworkReply::NoError) {
        const auto &data = m_reply->readAll();
        emit dataRetrieved(data, true);
    } else if (m_reply->error() == QNetworkReply::InsecureRedirectError) {
        const auto &location = m_reply->header(QNetworkRequest::LocationHeader);
        qDebug() << "redirecting to" << location;
        retrieveData(location.toUrl());
    } else {
        qDebug() << "Retriever error: " << m_reply->errorString();
        emit dataRetrieved({}, false);
    }
}
