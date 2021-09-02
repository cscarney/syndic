/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    IconProvider();
    ~IconProvider();
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    static void discoverIcon(FeedCore::Feed *feed);
private:
    QSharedPointer<QNetworkAccessManager> m_nam;
};



#endif // ICONPROVIDER_H
