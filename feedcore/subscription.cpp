/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "subscription.h"
using namespace FeedCore;

struct Subscription::PrivData {
    QUrl url;
    QUrl link;
    UpdateMode updateMode{InheritUpdateMode};
    time_t updateInterval{0};
    UpdateMode expireMode{InheritUpdateMode};
    qint64 expireAge{0};
    int flags{0};
};

Subscription::Subscription(QObject *parent)
    : Feed(parent)
    , d{std::make_unique<PrivData>()}
{
}

Subscription::~Subscription() = default;

void Subscription::update(const QDateTime &timestamp)
{
    updater()->start(timestamp);
}

void Subscription::cancelUpdates()
{
    updater()->abort();
}

const QUrl &Subscription::url() const
{
    return d->url;
}

void Subscription::setUrl(const QUrl &url)
{
    if (d->url != url) {
        d->url = url;
        emit urlChanged();
    }
}

const QUrl &Subscription::link()
{
    return d->link;
}

void Subscription::setLink(const QUrl &link)
{
    if (d->link != link) {
        d->link = link;
        emit linkChanged();
    }
}

Subscription::UpdateMode Subscription::updateMode()
{
    return d->updateMode;
}

void Subscription::setUpdateMode(Subscription::UpdateMode updateMode)
{
    if (updateMode != d->updateMode) {
        d->updateMode = updateMode;
        emit updateModeChanged();
    }
}

qint64 Subscription::updateInterval()
{
    return d->updateInterval;
}

void Subscription::setUpdateInterval(qint64 updateInterval)
{
    if (updateInterval != d->updateInterval) {
        d->updateInterval = updateInterval;
        emit updateIntervalChanged();
    }
}

Subscription::UpdateMode Subscription::expireMode()
{
    return d->expireMode;
}

void Subscription::setExpireMode(Subscription::UpdateMode expireMode)
{
    if (d->expireMode != expireMode) {
        d->expireMode = expireMode;
        emit expireModeChanged();
    }
}

void Subscription::setExpireAge(qint64 expireAge)
{
    if (expireAge != d->expireAge) {
        d->expireAge = expireAge;
        emit expireAgeChanged();
    }
}

qint64 Subscription::expireAge()
{
    return d->expireAge;
}

void Subscription::setFlags(int flags)
{
    if (flags != d->flags) {
        d->flags = flags;
        emit flagsChanged();
    }
}

int Subscription::flags() const
{
    return d->flags;
}

void Subscription::updateParams(Subscription *other)
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
    setFlags(other->flags());
}

void Subscription::requestDelete()
{
    emit deleteRequested();
}

struct Subscription::Updater::PrivData {
    Subscription *feed;
    QDateTime updateStartTime;
    QString errorMsg;
    explicit PrivData(Subscription *feed)
        : feed(feed){};
};

Subscription::Updater::Updater(Subscription *feed, QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>(feed)}
{
}

Subscription::Updater::~Updater() = default;

void Subscription::Updater::start(const QDateTime &timestamp)
{
    d->updateStartTime = timestamp;
    if (d->feed->status() != LoadStatus::Updating) {
        d->feed->setStatus(LoadStatus::Updating);
        run();
    }
}

QString Subscription::Updater::error()
{
    return d->errorMsg;
}

Subscription *Subscription::Updater::feed()
{
    return d->feed;
}

const QDateTime &Subscription::Updater::updateStartTime()
{
    return d->updateStartTime;
}

void Subscription::Updater::finish()
{
    d->feed->setLastUpdate(d->updateStartTime);
    d->feed->setStatus(LoadStatus::Idle);
    cleanup();
}

void Subscription::Updater::setError(const QString &errorMsg)
{
    d->errorMsg = errorMsg;
    d->feed->setStatus(LoadStatus::Error);
    cleanup();
}

void Subscription::Updater::aborted()
{
    d->feed->setStatus(LoadStatus::Idle);
    cleanup();
}

void Subscription::Updater::cleanup()
{
}
