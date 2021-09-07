#include "networkaccessmanagerfactory.h"
#include "networkaccessmanager.h"

NetworkAccessManagerFactory::NetworkAccessManagerFactory() = default;

QNetworkAccessManager *NetworkAccessManagerFactory::create(QObject *parent)
{
    return new FeedCore::NetworkAccessManager(parent);
}
