#ifndef XMLFEEDUPDATER_H
#define XMLFEEDUPDATER_H

#include "feedupdater.h"
#include <QUrl>
#include <Syndication/Loader>

namespace FeedCore {

class XMLFeedUpdater : public FeedUpdater
{
public:
    XMLFeedUpdater(Feed *feed, time_t updateInterval, time_t lastUpdate, QObject *parent);
    void run() final;

private:
    void loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status);
};

}

#endif // XMLFEEDUPDATER_H
