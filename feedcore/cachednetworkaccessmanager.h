#ifndef CACHEDNETWORKACCESSMANAGER_H
#define CACHEDNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

namespace FeedCore {
class CachedNetworkAccessManager : public QNetworkAccessManager
{
public:
    CachedNetworkAccessManager();
};
}

#endif // CACHEDNETWORKACCESSMANAGER_H
