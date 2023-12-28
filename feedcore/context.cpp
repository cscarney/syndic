/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "context.h"
#include "article.h"
#include "automation/automationengine.h"
#include "categoryfeed.h"
#include "cmake-config.h"
#include "feed.h"
#include "future.h"
#include "opmlreader.h"
#include "provisionalfeed.h"
#include "scheduler.h"
#include "storage.h"
#include <QDebug>
#include <QFile>
#include <QNetworkInformation>
#include <QSet>

#ifdef QReadable_FOUND
#include "readability/qreadablereadability.h"
using ReadabilityType = FeedCore::QReadableReadability;
#else
#include "readability/placeholderreadability.h"
using ReadabilityType = FeedCore::PlaceholderReadability;
#endif

namespace FeedCore
{
struct Context::PrivData {
    enum ContextFlags { FeedListComplete = 1, UpdateRequestPending = 1U << 1U, PrefetchReadableContent = 1U << 2U, FeedsScheduledByDefault = 1U << 3U };

    Context *parent;
    Storage *storage;
    QSet<Feed *> feeds;
    qint64 updateInterval{0};
    qint64 expireAge{0};
    Scheduler *updateScheduler;
    Readability *readability{nullptr};
    QFlags<ContextFlags> flags{};
    QWeakPointer<Feed> allItemsFeed{nullptr};

    PrivData(Storage *storage, Context *parent);
    void configureUpdates(Feed *feed, const QDateTime &timestamp = QDateTime::currentDateTime()) const;
    void configureExpiration(Feed *feed) const;
};

namespace
{
class AllItemsFeed : public AggregateFeed
{
public:
    explicit AllItemsFeed(Context *context, QObject *parent = nullptr);
    QFuture<ArticleRef> getArticles(bool unreadFilter) final;

private:
    Context *m_context{nullptr};
};
}

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>(storage, this)}
{
    if (QNetworkInformation::loadDefaultBackend()) {
        QObject::connect(QNetworkInformation::instance(), &QNetworkInformation::reachabilityChanged, d->updateScheduler, &Scheduler::clearErrors);
        QObject::connect(QNetworkInformation::instance(), &QNetworkInformation::isBehindCaptivePortalChanged, d->updateScheduler, &Scheduler::clearErrors);
    }

    QFuture<Feed *> getFeeds{d->storage->getFeeds()};
    Future::safeThen(getFeeds, this, [this](auto &getFeeds) {
        populateFeeds(Future::safeResults(getFeeds));
    });
    AutomationEngine::fromDefaultConfigFile(this);
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
        shouldSchedule = flags.testFlag(FeedsScheduledByDefault);
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

QSharedPointer<Feed> Context::allItemsFeed()
{
    QSharedPointer<Feed> result = d->allItemsFeed;
    if (!result) {
        result.reset(new AllItemsFeed(this));
        d->allItemsFeed = result;
    }
    return result;
}

QSet<Feed *> Context::getCategoryFeeds(const QString &category)
{
    QSet<Feed *> result;
    for (auto *f : std::as_const(d->feeds)) {
        if (f->category() == category) {
            result << f;
        }
    }
    return result;
}

Feed *Context::createCategoryFeed(const QString &category)
{
    return new CategoryFeed(this, category);
}

QFuture<ArticleRef> Context::searchArticles(const QString &query)
{
    return d->storage->getSearchResults(query);
}

void Context::addFeed(ProvisionalFeed *feed)
{
    QFuture<Feed *> q{d->storage->storeFeed(feed)};
    Future::safeThen(q, this, [this, feed = QPointer(feed)](auto &q) {
        const auto &result = Future::safeResults(q);
        registerFeeds(result);
        if (!feed.isNull()) {
            if (result.isEmpty()) {
                // TODO report backend errors
                emit feed->saveFailed();
            } else {
                feed->setTargetFeed(result.first());
            }
        }
    });
}

QStringList Context::getCategories()
{
    QMap<QString, std::nullptr_t> categories{{"", nullptr}};
    for (auto *feed : std::as_const(d->feeds)) {
        categories.insert(feed->category(), nullptr);
    }
    return categories.keys();
}

QFuture<ArticleRef> Context::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return d->storage->getUnread();
    }
    return d->storage->getAll();
}

QFuture<ArticleRef> Context::getStarred()
{
    return d->storage->getStarred();
}

QFuture<ArticleRef> Context::getHighlights()
{
    return d->storage->getHighlights();
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
    for (Feed *feed : std::as_const(d->feeds)) {
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
    for (Feed *feed : std::as_const(d->feeds)) {
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
    QList<Feed *> uncategorizedFeeds;
    QMap<QString, QList<Feed *>> categories;
    for (Feed *feed : std::as_const(d->feeds)) {
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
    for (Feed *feed : std::as_const(uncategorizedFeeds)) {
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
        auto q = d->storage->storeFeed(feed);
        Future::safeThen(q, this, [this, opml](auto &q) {
            registerFeeds(Future::safeResults(q));
        });
    }
    QObject::connect(opml.get(), &QObject::destroyed, this, [this] {
        d->updateScheduler->start();
    });
}

Readability *Context::getReadability()
{
    if (d->readability == nullptr) {
        d->readability = new ReadabilityType();
        d->readability->setParent(this);
    }
    return d->readability;
}

bool Context::defaultUpdateEnabled() const
{
    return d->flags.testAnyFlag(PrivData::FeedsScheduledByDefault);
}

void Context::setDefaultUpdateEnabled(bool defaultUpdateEnabled)
{
    if (Context::defaultUpdateEnabled() == defaultUpdateEnabled) {
        return;
    }

    d->flags.setFlag(PrivData::FeedsScheduledByDefault, defaultUpdateEnabled);
    const QDateTime timestamp = QDateTime::currentDateTime();
    for (Feed *feed : std::as_const(d->feeds)) {
        if (feed->updateMode() == Feed::InheritUpdateMode) {
            d->configureUpdates(feed, timestamp);
        }
    }
    emit defaultUpdateEnabledChanged();
}

void Context::populateFeeds(const QList<Feed *> &feeds)
{
    registerFeeds(feeds);
    d->flags.setFlag(PrivData::FeedListComplete);
    emit feedListPopulated(d->feeds.size());
}

void Context::registerFeeds(const QList<Feed *> &feeds)
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

bool Context::prefetchContent() const
{
    return d->flags.testFlag(PrivData::PrefetchReadableContent);
}

void Context::setPrefetchContent(bool newPrefetchContent)
{
    if (prefetchContent() == newPrefetchContent) {
        return;
    }
    d->flags.setFlag(PrivData::PrefetchReadableContent, newPrefetchContent);
    emit prefetchContentChanged();
}

AllItemsFeed::AllItemsFeed(Context *context, QObject *parent)
    : AggregateFeed(parent)
    , m_context{context}
{
    for (const auto &feed : context->getFeeds()) {
        addFeed(feed);
    }
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

QFuture<ArticleRef> AllItemsFeed::getArticles(bool unreadFilter)
{
    return m_context->getArticles(unreadFilter);
}
}
