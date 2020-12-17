#include "dataretriever.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace FeedCore {

DataRetriever::DataRetriever() = default;

void DataRetriever::retrieveData(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "curl/4.6");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    m_reply = m_nam.get(request);
    m_reply->setParent(this);
    QObject::connect(m_reply, &QNetworkReply::finished, this, &DataRetriever::onFinished);
}

int DataRetriever::errorCode() const
{
    return 0;
}

void DataRetriever::abort()
{
    // TODO implement this
}

void DataRetriever::onFinished()
{
    if (m_reply->error() == QNetworkReply::NoError) {
        const auto &data = m_reply->readAll();
        emit dataRetrieved(data, true);
    } else if (m_reply->error() == QNetworkReply::InsecureRedirectError) {
        const auto &location = m_reply->header(QNetworkRequest::LocationHeader);
        qDebug() << "insecure redirect to" << location;
        retrieveData(location.toUrl());
    } else {
        qDebug() << "Failed to get data:"<<m_reply->errorString();
        emit dataRetrieved({}, false);
    }
}

}
