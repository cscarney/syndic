/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "context.h"
#include "feed.h"
#include "future.h"
#include "opmlreader.h"
#include "provisionalfeed.h"
#include "readability/qreadablereadability.h"
#include "scheduler.h"
#include "storage.h"
#include <QDebug>
#include <QFile>
#include <QNetworkConfigurationManager>
#include <QSet>

namespace FeedCore
{
struct Context::PrivData {
    Context *parent;
    Storage *storage;
    QSet<Feed *> feeds;
    bool defaultUpdate{false};
    qint64 updateInterval{0};
    qint64 expireAge{0};
    Scheduler *updateScheduler;
    QNetworkConfigurationManager ncm;
    Readability *readability{nullptr};

    PrivData(Storage *storage, Context *parent);
    void configureUpdates(Feed *feed, const QDateTime &timestamp = QDateTime::currentDateTime()) const;
    void configureExpiration(Feed *feed) const;
};

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>(storage, this)}
{
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationAdded, d->updateScheduler, &Scheduler::clearErrors);
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationChanged, d->updateScheduler, &Scheduler::clearErrors);

    Future<Feed *> *getFeeds{d->storage->getFeeds()};
    QObject::connect(getFeeds, &BaseFuture::finished, this, [this, getFeeds] {
        populateFeeds(getFeeds->result());
    });
    d->updateScheduler->start();
}

Context::~Context() = default;

Context::PrivData::PrivData(Storage *storage, Context *parent)
    : parent(parent)
    , storage(storage)
    , updateScheduler(new Scheduler(parent))
{
    storage->setParent(parent);
}

void Context::PrivData::configureUpdates(Feed *feed, const QDateTime &timestamp) const
{
    auto updateMode{feed->updateMode()};
    bool shouldSchedule{false};
    if (updateMode == Feed::InheritUpdateMode) {
        feed->setUpdateInterval(updateInterval);
        shouldSchedule = defaultUpdate;
    } else {
        shouldSchedule = (updateMode != Feed::DisableUpdateMode);
    }

    if (shouldSchedule) {
        updateScheduler->schedule(feed, timestamp);
    } else {
        updateScheduler->unschedule(feed);
    }
}

void Context::PrivData::configureExpiration(Feed *feed) const
{
    auto expireMode{feed->expireMode()};
    if (expireMode != Feed::OverrideUpdateMode) {
        feed->setExpireAge(expireAge);
    }
}

const QSet<Feed *> &Context::getFeeds()
{
    return d->feeds;
}

void Context::addFeed(ProvisionalFeed *feed)
{
    Future<Feed *> *q{d->storage->storeFeed(feed)};
    QObject::connect(q, &BaseFuture::finished, this, [this, q, feed = QPointer(feed)] {
        const auto &result = q->result();
        registerFeeds(result);
        if (!feed.isNull()) {
            if (q->result().isEmpty()) {
                // TODO report backend errors
                emit feed->saveFailed();
            } else {
                feed->setTargetFeed(q->result().first());
            }
        }
    });
}

QStringList Context::getCategories()
{
    QMap<QString, std::nullptr_t> categories{{"", nullptr}};
    for (auto *feed : qAsConst(d->feeds)) {
        categories.insert(feed->category(), nullptr);
    }
    return categories.keys();
}

Future<ArticleRef> *Context::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return d->storage->getUnread();
    }
    return d->storage->getAll();
}

Future<ArticleRef> *Context::getStarred()
{
    return d->storage->getStarred();
}

void Context::requestUpdate()
{
    const auto &timestamp = QDateTime::currentDateTime();
    const auto &feeds = d->feeds;
    for (Feed *const entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Context::abortUpdates()
{
    const auto &feeds = d->feeds;
    for (Feed *const entry : feeds) {
        entry->updater()->abort();
    }
}

qint64 Context::defaultUpdateInterval()
{
    return d->updateInterval;
}

void Context::setDefaultUpdateInterval(qint64 defaultUpdateInterval)
{
    if (d->updateInterval == defaultUpdateInterval) {
        return;
    }
    d->updateInterval = defaultUpdateInterval;
    for (Feed *feed : qAsConst(d->feeds)) {
        if (feed->updateMode() == Feed::InheritUpdateMode) {
            feed->setUpdateInterval(defaultUpdateInterval);
        }
    }
    emit defaultUpdateIntervalChanged();
}

qint64 Context::expireAge()
{
    return d->expireAge;
}

void Context::setExpireAge(qint64 expireAge)
{
    if (d->expireAge == expireAge) {
        return;
    }
    d->expireAge = expireAge;
    for (Feed *feed : qAsConst(d->feeds)) {
        if (feed->expireMode() != Feed::OverrideUpdateMode) {
            feed->setExpireAge(expireAge);
        }
    }
    emit expireAgeChanged();
}

static QString urlToPath(const QUrl &url)
{
    QString path(url.toLocalFile());
#ifdef ANDROID
    // TODO maybe Qt has a better way to do this?
    if (path.isEmpty()) {
        if (url.scheme() == "content") {
            path = QLatin1String("content:") + url.path();
        }
    }
#endif
    return path;
}

static void writeOpmlFeed(QXmlStreamWriter &xml, Feed *feed)
{
    xml.writeEmptyElement("outline");
    xml.writeAttribute("type", "rss");
    xml.writeAttribute("text", feed->name());
    xml.writeAttribute("xmlUrl", feed->url().toString());
}

void Context::exportOpml(const QUrl &url) const
{
    QFile file(urlToPath(url));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QVector<Feed *> uncategorizedFeeds;
    QMap<QString, QVector<Feed *>> categories;
    for (Feed *feed : qAsConst(d->feeds)) {
        QString category = feed->category();
        if (category.isEmpty()) {
            uncategorizedFeeds.append(feed);
        } else {
            categories[category].append(feed);
        }
    }

    QXmlStreamWriter xml(&file);
    xml.writeStartDocument();
    xml.writeStartElement("opml");
    xml.writeAttribute("version", "1.0");
    xml.writeStartElement("head");
    xml.writeEndElement();
    xml.writeStartElement("body");
    for (Feed *feed : qAsConst(uncategorizedFeeds)) {
        writeOpmlFeed(xml, feed);
    }
    for (auto i = categories.constBegin(); i != categories.constEnd(); ++i) {
        xml.writeStartElement("outline");
        xml.writeAttribute("text", i.key());
        for (Feed *feed : i.value()) {
            writeOpmlFeed(xml, feed);
        }
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    file.close();
}

void Context::importOpml(const QUrl &url)
{
    QFile file(urlToPath(url));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "failed to open file" << url;
        return;
    }
    QSharedPointer<OpmlReader> opml(new OpmlReader(&file, d->feeds));
    opml->readAll();
    file.close();

    if (opml->hasError()) {
        qDebug() << "failed to import OPML:" << opml->errorString();
        return;
    }

    for (ProvisionalFeed *feed : opml->updatedFeeds()) {
        feed->save();
    }

    d->updateScheduler->stop();
    for (ProvisionalFeed *feed : opml->newFeeds()) {
        auto *q = d->storage->storeFeed(feed);
        QObject::connect(q, &BaseFuture::finished, this, [this, opml, q] {
            registerFeeds(q->result());
        });
    }
    QObject::connect(opml.get(), &QObject::destroyed, this, [this] {
        d->updateScheduler->start();
    });
}

Readability *Context::getReadability()
{
    if (d->readability == nullptr) {
        d->readability = new QReadableReadability();
        d->readability->setParent(this);
    }
    return d->readability;
}

bool Context::defaultUpdateEnabled() const
{
    return d->defaultUpdate;
}

void Context::setDefaultUpdateEnabled(bool defaultUpdateEnabled)
{
    if (d->defaultUpdate == defaultUpdateEnabled) {
        return;
    }

    d->defaultUpdate = defaultUpdateEnabled;
    const QDateTime timestamp = QDateTime::currentDateTime();
    for (Feed *feed : qAsConst(d->feeds)) {
        if (feed->updateMode() == Feed::InheritUpdateMode) {
            d->configureUpdates(feed, timestamp);
        }
    }
    emit defaultUpdateEnabledChanged();
}

void Context::populateFeeds(const QVector<Feed *> &feeds)
{
    registerFeeds(feeds);
    emit feedListPopulated(d->feeds.size());
}

void Context::registerFeeds(const QVector<Feed *> &feeds)
{
    const QDateTime timestamp = QDateTime::currentDateTime();
    for (const auto &feed : feeds) {
        d->feeds.insert(feed);
        d->configureExpiration(feed);
        d->configureUpdates(feed, timestamp);
        QObject::connect(feed, &QObject::destroyed, this, [this, feed] {
            d->feeds.remove(feed);
        });
        QObject::connect(feed, &Feed::updateModeChanged, this, [this, feed] {
            d->configureUpdates(feed);
        });
        QObject::connect(feed, &Feed::expireModeChanged, this, [this, feed] {
            d->configureExpiration(feed);
        });
        emit feedAdded(feed);
    }
}

}
