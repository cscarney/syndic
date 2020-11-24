#ifndef XMLFEEDUPDATER_H
#define XMLFEEDUPDATER_H

#include "feedupdater.h"
#include <QUrl>
#include <Syndication/Loader>

class XMLFeedUpdater : public FeedUpdater
{
public:
    XMLFeedUpdater(qint64 feedId, QUrl url, time_t updateInterval, time_t lastUpdate, QObject *parent = nullptr);
    void run() override final;

private:
    QUrl m_url;

    void loadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);
};

#endif // XMLFEEDUPDATER_H
