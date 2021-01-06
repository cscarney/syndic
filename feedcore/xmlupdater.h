#ifndef XMLUPDATER_H
#define XMLUPDATER_H
#include "updater.h"
#include <QUrl>
#include <Syndication/Loader>

namespace FeedCore {
class XMLUpdater : public Updater
{
public:
    XMLUpdater(Feed *feed, time_t updateInterval, time_t lastUpdate, QObject *parent);
    void run() final;
private:
    void loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status);
};
}
#endif // XMLUPDATER_H
