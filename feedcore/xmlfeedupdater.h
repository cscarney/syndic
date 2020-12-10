#ifndef XMLFEEDUPDATER_H
#define XMLFEEDUPDATER_H

#include "feedupdater.h"
#include <QUrl>
#include <Syndication/Loader>

namespace FeedCore {

class XMLFeedUpdater : public FeedUpdater
{
public:
    XMLFeedUpdater(const FeedRef &feed, time_t updateInterval, time_t lastUpdate, QObject *parent = nullptr);
    void run() override final;

private:
    void loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &feed, Syndication::ErrorCode status);
};

}

#endif // XMLFEEDUPDATER_H
