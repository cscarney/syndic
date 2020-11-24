#include "xmlfeedupdater.h"
#include <Syndication/Loader>
#include "dataretriever.h"

XMLFeedUpdater::XMLFeedUpdater(qint64 feedId, QUrl url, time_t updateInterval, time_t lastUpdate, QObject *parent):
    FeedUpdater(feedId, updateInterval, lastUpdate, parent),
    m_url(url)
{

}

void XMLFeedUpdater::run()
{
    Syndication::Loader *loader = Syndication::Loader::create();
    QObject::connect(loader, &Syndication::Loader::loadingComplete, this, &XMLFeedUpdater::loadingComplete);
    loader->loadFrom(m_url, new DataRetriever);
}

void XMLFeedUpdater::loadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
{
    switch (status) {
    case Syndication::Success:
        finish(feed);
        break;
    case Syndication::Aborted:
        qDebug() << "load aborted for " << m_url;
        break;
    case Syndication::Timeout:
        qDebug() << "timeout loading " << m_url;
        break;
    case Syndication::UnknownHost:
        qDebug() << "unknown host " << m_url.host();
        break;
    case Syndication::FileNotFound:
        qDebug() << "file not found " << m_url;
        break;
    case Syndication::OtherRetrieverError:
        qDebug() << "other retriever error" << m_url;
        break;
    case Syndication::InvalidXml:
        qDebug() << "invalid xml" << m_url;
        break;
    case Syndication::XmlNotAccepted:
        qDebug() << "xml not accepted" << m_url;
        break;
    case Syndication::InvalidFormat:
        qDebug() << "invalid format" << m_url;
        break;
    default:
        qDebug() << "unknown error loading " << m_url;
        setError(tr("unknown error", "error message"));
    }
}
