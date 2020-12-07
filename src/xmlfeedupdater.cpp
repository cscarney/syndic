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
    QString errorMessage;
    switch (status) {
    case Syndication::Success:
        finish(feed);
        return;
    case Syndication::Aborted:
        qDebug() << "load aborted for " << m_url;
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
        qDebug() << "unknown error loading " << m_url;
        errorMessage = tr("Unknown Error", "error message");
    }

    // try the discovered url
    if (loader->discoveredFeedURL().isValid()) {
        m_url = loader->discoveredFeedURL();
        run();
    } else {
        setError(errorMessage);
    }
}
