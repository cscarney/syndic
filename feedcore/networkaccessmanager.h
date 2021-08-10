#ifndef FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
#define FEEDCORE_CACHEDNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

namespace FeedCore {
class NetworkAccessManager : public QNetworkAccessManager
{
public:
    static NetworkAccessManager *instance() {
        static NetworkAccessManager *singleton = new NetworkAccessManager();
        return singleton;
    }
private:
    NetworkAccessManager();
};
}

#endif // FEEDCORE_CACHEDNETWORKACCESSMANAGER_H
