/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "notificationcontroller.h"
#include "article.h"
#include "cmake-config.h"
#include "context.h"

#ifdef Qt5Widgets_FOUND
#define HAVE_SYSTEM_TRAY
#include <QGuiApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#endif

using namespace FeedCore;

struct NotificationController::PrivData {
  QSharedPointer<Feed> feed{nullptr};
  int counter{0};

#ifdef HAVE_SYSTEM_TRAY
    std::unique_ptr<QMenu> trayMenu;
    QSystemTrayIcon *trayIcon{nullptr};
#endif
};

NotificationController::NotificationController(FeedCore::Context *context, QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>()}
{
    d->feed = context->allItemsFeed();
    QObject::connect(d->feed.get(), &Feed::statusChanged, this,
                     &NotificationController::onStatusChanged);
    QObject::connect(d->feed.get(), &Feed::articleAdded, this,
                     &NotificationController::onArticleAdded);

#ifdef HAVE_SYSTEM_TRAY
    d->trayMenu = std::make_unique<QMenu>();
    d->trayMenu->addAction(tr("Quit", "system tray menu action"), this, [] {
        QCoreApplication::quit();
    });
    d->trayIcon = new QSystemTrayIcon(QIcon::fromTheme("com.rocksandpaper.syndic.tray-icon-symbolic"), this);
    d->trayIcon->setContextMenu(d->trayMenu.get());
    QObject::connect(d->trayIcon, &QSystemTrayIcon::activated, this, &NotificationController::activate);
    d->trayIcon->setToolTip(QGuiApplication::applicationDisplayName());
    d->trayIcon->show();
#endif
}

NotificationController::~NotificationController() = default;

void NotificationController::onStatusChanged()
{
    if (d->counter > 0) {
        postNotification();
    }
    d->counter = 0;
}

void NotificationController::onArticleAdded(const ArticleRef &article)
{
    if ((d->feed->status() == LoadStatus::Updating) && (!article->isRead())) {
        ++d->counter;
    }
}

void NotificationController::postNotification() const
{
    QString notificationText{tr("%1 New Item(s)", "Notification Text", d->counter)};
#ifdef HAVE_SYSTEM_TRAY
    d->trayIcon->setToolTip(notificationText);
#endif
}
