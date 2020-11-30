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
        setError(tr("Timeout", "error message"));
        break;
    case Syndication::UnknownHost:
        setError(tr("Unknown Host", "error message"));
        break;
    case Syndication::FileNotFound:
        setError(tr("File Not Found", "error message"));
        break;
    case Syndication::OtherRetrieverError:
        setError(tr("Retriever Error", "error message"));
        break;
    case Syndication::InvalidXml:
        setError(tr("Invalid XML", "error message"));
        break;
    case Syndication::XmlNotAccepted:
        setError(tr("XML Not Accepted", "error message"));
        break;
    case Syndication::InvalidFormat:
        setError(tr("Invalid Format", "error message"));
        break;
    default:
        qDebug() << "unknown error loading " << m_url;
        setError(tr("Unknown Error", "error message"));
    }
}
