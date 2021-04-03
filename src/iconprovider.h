#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H
#include <QQuickAsyncImageProvider>
#include <QNetworkAccessManager>

namespace FeedCore {
    class Feed;
    class Context;
}

class IconProvider : public QQuickAsyncImageProvider
{
public:
    IconProvider(FeedCore::Context *ctx);
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    static void discoverIcon(QNetworkAccessManager *nam, FeedCore::Feed *feed);

private:
    QNetworkAccessManager *m_nam;
};



#endif // ICONPROVIDER_H
