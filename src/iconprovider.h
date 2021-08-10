#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H
#include <QQuickAsyncImageProvider>

namespace FeedCore {
    class Feed;
    class Context;
}

class IconProvider : public QQuickAsyncImageProvider
{
public:
    IconProvider(FeedCore::Context *ctx);
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    static void discoverIcon(FeedCore::Feed *feed);
};



#endif // ICONPROVIDER_H
