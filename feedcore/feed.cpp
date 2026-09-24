/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feed.h"
#include <QTimer>
using namespace FeedCore;

struct Feed::PrivData {
    QString name;
    QString category;
    QUrl icon;
    int unreadCount{0};
    int pendingUnreadCountChange{0};
    LoadStatus status{LoadStatus::Idle};
    QDateTime lastUpdate;
};

Feed::Feed(QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>()}
{
}

Feed::~Feed()
{
    setUnreadCount(0);
}

void Feed::update(const QDateTime & /*timestamp*/)
{
}

void Feed::cancelUpdates()
{
}

const QString &Feed::name() const
{
    return d->name;
}

void Feed::setName(const QString &name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged();
    }
}

void Feed::setUnreadCount(int unreadCount)
{
    if (d->unreadCount != unreadCount) {
        int delta = unreadCount - d->unreadCount;
        d->unreadCount = unreadCount;
        emit unreadCountChanged(delta);
    }
}

void Feed::incrementUnreadCount(int delta)
{
    if (d->pendingUnreadCountChange == 0) {
        QTimer::singleShot(0, this, [this] {
            if (d->pendingUnreadCountChange != 0) {
                d->unreadCount += d->pendingUnreadCountChange;
                emit unreadCountChanged(d->pendingUnreadCountChange);
                d->pendingUnreadCountChange = 0;
            }
        });
    }
    d->pendingUnreadCountChange += delta;
}

void Feed::decrementUnreadCount()
{
    incrementUnreadCount(-1);
}

QString Feed::category() const
{
    return d->category;
}

void Feed::setCategory(const QString &category)
{
    if (d->category != category) {
        d->category = category;
        emit categoryChanged();
    }
}

const QUrl &Feed::icon()
{
    return d->icon;
}

void Feed::setIcon(const QUrl &icon)
{
    if (!icon.isValid()) {
        return;
    }
    if (d->icon != icon) {
        d->icon = icon;
        emit iconChanged();
    }
}

int Feed::unreadCount() const
{
    return d->unreadCount;
}

LoadStatus Feed::status() const
{
    return d->status;
}

void Feed::setStatus(LoadStatus status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged();
    }
}

const QDateTime &Feed::lastUpdate()
{
    return d->lastUpdate;
}

void Feed::setLastUpdate(const QDateTime &lastUpdate)
{
    if (d->lastUpdate != lastUpdate) {
        d->lastUpdate = lastUpdate;
        emit lastUpdateChanged();
    }
}
