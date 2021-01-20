#include "xmlupdater.h"
#include <Syndication/Loader>
#include "dataretriever.h"

namespace FeedCore {

XMLUpdater::XMLUpdater(Feed *feed, QObject *parent):
    Updater(feed, parent)
{

}

void XMLUpdater::run()
{
    if (!feed()->url().isValid()) {
        setError(tr("Invalid URL", "error message"));
        return;
    }
    m_loader = Syndication::Loader::create();
    QObject::connect(m_loader, &Syndication::Loader::loadingComplete, this, &XMLUpdater::loadingComplete);
    m_loader->loadFrom(feed()->url(), new DataRetriever);
}

void XMLUpdater::abort()
{
    Syndication::Loader *loader = m_loader;
    if (loader){
        loader->abort();
    }
}

void XMLUpdater::loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status)
{
    m_loader = nullptr;
    Feed *feed { this->feed() };
    QString errorMessage;
    switch (status) {
    case Syndication::Success:
        feed->updateFromSource(content);
        finish();
        return;
    case Syndication::Aborted:
        qDebug() << "load aborted for " << feed->url();
        finish();
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
