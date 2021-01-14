#ifndef XMLUPDATER_H
#define XMLUPDATER_H
#include "updater.h"
#include <QUrl>
#include <Syndication/Loader>

namespace FeedCore {
class XMLUpdater : public Updater
{
public:
    XMLUpdater(Feed *feed, QObject *parent);
    void run() final;
    void abort() final;
private:
    Syndication::Loader *m_loader { nullptr };
    void loadingComplete(Syndication::Loader *loader, const Syndication::FeedPtr &content, Syndication::ErrorCode status);
};
}
#endif // XMLUPDATER_H
