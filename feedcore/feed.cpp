/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feed.h"
using namespace FeedCore;

struct Feed::PrivData {
    QString name;
    QString category;
    QUrl url;
    QUrl link;
    QUrl icon;
    int unreadCount{0};
    int pendingUnreadCountChange{0};
    LoadStatus status{LoadStatus::Idle};
    UpdateMode updateMode{InheritUpdateMode};
    time_t updateInterval{0};
    UpdateMode expireMode{InheritUpdateMode};
    qint64 expireAge{0};
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

const QUrl &Feed::url() const
{
    return d->url;
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

void Feed::setUrl(const QUrl &url)
{
    if (d->url != url) {
        d->url = url;
        emit urlChanged();
    }
}

const QUrl &Feed::link()
{
    return d->link;
}

void Feed::setLink(const QUrl &link)
{
    if (d->link != link) {
        d->link = link;
        emit linkChanged();
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

Feed::UpdateMode Feed::updateMode()
{
    return d->updateMode;
}

void Feed::setUpdateMode(Feed::UpdateMode updateMode)
{
    if (updateMode != d->updateMode) {
        d->updateMode = updateMode;
        emit updateModeChanged();
    }
}

qint64 Feed::updateInterval()
{
    return d->updateInterval;
}

void Feed::setUpdateInterval(qint64 updateInterval)
{
    if (updateInterval != d->updateInterval) {
        d->updateInterval = updateInterval;
        emit updateIntervalChanged();
    }
}

Feed::UpdateMode Feed::expireMode()
{
    return d->expireMode;
}

void Feed::setExpireMode(Feed::UpdateMode expireMode)
{
    if (d->expireMode != expireMode) {
        d->expireMode = expireMode;
        emit expireModeChanged();
    }
}

void Feed::setExpireAge(qint64 expireAge)
{
    if (expireAge != d->expireAge) {
        d->expireAge = expireAge;
        emit expireAgeChanged();
    }
}

qint64 Feed::expireAge()
{
    return d->expireAge;
}

void Feed::updateParams(Feed *other)
{
    if (other == nullptr) {
        return;
    }
    setName(other->name());
    setCategory(other->category());
    setUrl(other->url());
    setUpdateInterval(other->updateInterval());
    setUpdateMode(other->updateMode());
    setExpireAge(other->expireAge());
    setExpireMode(other->expireMode());
}

bool Feed::editable()
{
    return false;
}

void Feed::requestDelete()
{
    emit deleteRequested();
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

struct Feed::Updater::PrivData {
    Feed *feed;
    QDateTime updateStartTime;
    QString errorMsg;
    bool active{false};
    explicit PrivData(Feed *feed)
        : feed(feed){};
};

Feed::Updater::Updater(Feed *feed, QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>(feed)}
{
}

Feed::Updater::~Updater() = default;

void Feed::Updater::start(const QDateTime &timestamp)
{
    d->updateStartTime = timestamp;
    if (d->feed->status() != LoadStatus::Updating) {
        d->feed->setStatus(LoadStatus::Updating);
        run();
    }
}

QString Feed::Updater::error()
{
    return d->errorMsg;
}

Feed *Feed::Updater::feed()
{
    return d->feed;
}

const QDateTime &Feed::Updater::updateStartTime()
{
    return d->updateStartTime;
}

void Feed::Updater::finish()
{
    d->feed->setLastUpdate(d->updateStartTime);
    d->feed->setStatus(LoadStatus::Idle);
}

void Feed::Updater::setError(const QString &errorMsg)
{
    d->errorMsg = errorMsg;
    d->feed->setStatus(LoadStatus::Error);
}

void Feed::Updater::aborted()
{
    d->feed->setStatus(LoadStatus::Idle);
}
