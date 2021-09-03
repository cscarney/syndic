/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NOTIFICATIONCONTROLLER_H
#define NOTIFICATIONCONTROLLER_H

#include <QObject>
#include "feed.h"
namespace FeedCore {
    class Context;
}

class NotificationController : public QObject
{
    Q_OBJECT
public:
    explicit NotificationController(FeedCore::Context *context, QObject *parent = nullptr);
    ~NotificationController();

signals:
    void activate();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void onStatusChanged();
    void onArticleAdded(const FeedCore::ArticleRef &article);
    void postNotification() const;
};

#endif // NOTIFICATIONCONTROLLER_H
