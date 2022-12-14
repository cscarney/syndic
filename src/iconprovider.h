/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QMutex>
#include <QQuickAsyncImageProvider>

namespace FeedCore
{
class Feed;
class Context;
}

/**
 * The image provider for feed icons.
 *
 * Requests for feed icon URLs are routed through this class so that they
 * can be cached more aggressvely than normal network content.
 *
 * TODO This class currently also handles icon discovery, but that should probably
 * be moved to core.
 */
class IconProvider : public QObject, public QQuickAsyncImageProvider
{
    Q_OBJECT
public:
    IconProvider();
    ~IconProvider();
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    static void discoverIcon(FeedCore::Feed *feed);

private:
    class IconImageResponse;
    class IconImageEntry;
    IconImageEntry *getEntry(const QUrl &url);

    static IconProvider *s_instance;
    QSharedPointer<QNetworkAccessManager> m_nam;
    QHash<QUrl, IconImageEntry *> m_icons;
};
