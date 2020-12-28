#include "xmlfeedupdater.h"

#include <Syndication/Loader>

#include "dataretriever.h"

namespace FeedCore {

XMLFeedUpdater::XMLFeedUpdater(Feed *feed, time_t updateInterval, time_t lastUpdate, QObject *parent):
    FeedUpdater(feed, updateInterval, lastUpdate, parent)
{

}

void XMLFeedUpdater::run()
{
    Syndication::Loader *loader { Syndication::Loader::create() };
    QObject::connect(loader, &Syndication::Loader::loadingComplete, this, &XMLFeedUpdater::loadingComplete);
    loader->loadFrom(feed()->url(), new DataRetriever);
}

void XMLFeedUpdater::loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status)
{
    Feed *feed { this->feed() };
    QString errorMessage;
    switch (status) {
    case Syndication::Success:
        feed->updateFromSource(content);
        finish();
        return;
    case Syndication::Aborted:
        qDebug() << "load aborted for " << feed->url();
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
        qDebug() << "unknown error loading " << feed->url();
        errorMessage = tr("Unknown Error", "error message");
    }

    // try the discovered url
    if (loader->discoveredFeedURL().isValid()) {
        feed->setUrl(loader->discoveredFeedURL());
        run();
    } else {
        setError(errorMessage);
    }
}

}
