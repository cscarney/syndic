/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "saveapi.h"
#include "article.h"
#include "context.h"
#include "pagesaver.h"
#include "subscription.h"
#include <QJsonArray>
#include <algorithm>

using namespace FeedCore;

namespace
{
QFuture<SaveApi::Response> ready(const SaveApi::Response &response)
{
    return QtFuture::makeReadyValueFuture(response);
}

QString feedId(const Subscription *feed)
{
    return feed ? feed->url().toString() : QString();
}

QJsonValue feedIdValue(const Feed *feed)
{
    const auto *subscription = qobject_cast<const Subscription *>(feed);
    return subscription ? QJsonValue(feedId(subscription)) : QJsonValue(QJsonValue::Null);
}

QJsonObject articleJson(const ArticleRef &article)
{
    return {
        {QStringLiteral("title"), article->title()},
        {QStringLiteral("url"), article->url().toString()},
        {QStringLiteral("feed"), feedIdValue(article->feed())},
        {QStringLiteral("feedName"), qobject_cast<Subscription *>(article->feed()) ? QJsonValue(article->feed()->name()) : QJsonValue(QJsonValue::Null)},
    };
}

QUrl requestUrl(const QJsonObject &body)
{
    const QUrl url(body.value(QStringLiteral("url")).toString(), QUrl::StrictMode);
    if (!url.isValid() || url.isRelative() || url.host().isEmpty()) {
        return {};
    }
    return url;
}
}

SaveApi::SaveApi(Context *context, PageSaver *saver, QObject *parent)
    : QObject(parent)
    , m_context(context)
    , m_saver(saver)
{
}

SaveApi::Response SaveApi::error(int status, const QString &code)
{
    return {status, {{QStringLiteral("error"), code}}};
}

static SaveApi::Response saveError(PageSaver::Error error)
{
    switch (error) {
    case PageSaver::InvalidUrl:
        return SaveApi::error(400, QStringLiteral("invalid-url"));
    case PageSaver::FeedNotFound:
        return SaveApi::error(400, QStringLiteral("invalid-feed"));
    case PageSaver::ExtractionFailed:
        return SaveApi::error(422, QStringLiteral("extraction-failed"));
    case PageSaver::SubscribeFailed:
        return SaveApi::error(500, QStringLiteral("subscribe-failed"));
    case PageSaver::StorageFailed:
    case PageSaver::NoError:
        break;
    }
    return SaveApi::error(500, QStringLiteral("save-failed"));
}

SaveApi::Response SaveApi::ping(bool paired)
{
    return {200,
            {
                {QStringLiteral("app"), QStringLiteral("syndic")},
                {QStringLiteral("apiVersion"), kApiVersion},
                {QStringLiteral("paired"), paired},
            }};
}

QJsonArray SaveApi::feedList() const
{
    QList<Subscription *> feeds{m_context->getFeeds().begin(), m_context->getFeeds().end()};
    std::sort(feeds.begin(), feeds.end(), [](Subscription *a, Subscription *b) {
        const int byCategory = QString::compare(a->category(), b->category(), Qt::CaseInsensitive);
        if (byCategory != 0) {
            return byCategory < 0;
        }
        return QString::compare(a->name(), b->name(), Qt::CaseInsensitive) < 0;
    });
    QJsonArray result;
    for (Subscription *feed : std::as_const(feeds)) {
        result.append(QJsonObject{
            {QStringLiteral("id"), feedId(feed)},
            {QStringLiteral("name"), feed->name()},
            {QStringLiteral("category"), feed->category()},
        });
    }
    return result;
}

QFuture<SaveApi::Response> SaveApi::resolve(const QJsonObject &body)
{
    const QUrl url = requestUrl(body);
    if (url.isEmpty()) {
        return ready(error(400, QStringLiteral("invalid-url")));
    }

    return m_saver->resolve(url).then(this, [this](const PageSaver::Resolution &resolution) {
        QJsonObject result{{QStringLiteral("feeds"), feedList()}};
        if (resolution.article) {
            result.insert(QStringLiteral("article"), articleJson(resolution.article));
        }
        if (resolution.suggestedFeed) {
            result.insert(QStringLiteral("suggestedFeed"), feedId(resolution.suggestedFeed));
        }
        if (!resolution.discovered.url.isEmpty()) {
            result.insert(QStringLiteral("discovered"),
                          QJsonObject{
                              {QStringLiteral("url"), resolution.discovered.url.toString()},
                              {QStringLiteral("name"), resolution.discovered.name},
                              {QStringLiteral("isWebPage"), resolution.discovered.isWebPage},
                          });
        }
        return Response{200, result};
    });
}

QFuture<SaveApi::Response> SaveApi::save(const QJsonObject &body)
{
    PageSaver::Request request;
    request.url = requestUrl(body);
    if (request.url.isEmpty()) {
        return ready(error(400, QStringLiteral("invalid-url")));
    }
    request.html = body.value(QStringLiteral("html")).toString();
    request.title = body.value(QStringLiteral("title")).toString().trimmed();
    request.author = body.value(QStringLiteral("author")).toString().trimmed();

    const QJsonValue subscribe = body.value(QStringLiteral("subscribe"));
    const QJsonValue feed = body.value(QStringLiteral("feed"));
    if (subscribe.isObject()) {
        const QJsonObject subscribeObject = subscribe.toObject();
        request.target = PageSaver::Request::SubscribeToFeed;
        request.subscribe.url = QUrl(subscribeObject.value(QStringLiteral("url")).toString(), QUrl::StrictMode);
        request.subscribe.name = subscribeObject.value(QStringLiteral("name")).toString();
        if (!request.subscribe.url.isValid() || request.subscribe.url.isRelative()) {
            return ready(error(400, QStringLiteral("invalid-feed")));
        }
    } else if (feed.isString()) {
        request.target = PageSaver::Request::ExistingFeed;
        request.feed = m_saver->feedByUrl(QUrl(feed.toString()));
        if (request.feed == nullptr) {
            return ready(error(400, QStringLiteral("invalid-feed")));
        }
    } else if (feed.isNull() || feed.isUndefined()) {
        request.target = PageSaver::Request::NoFeed;
    } else {
        return ready(error(400, QStringLiteral("invalid-feed")));
    }

    return m_saver->save(request).then(this, [](const PageSaver::Result &result) {
        if (!result.article) {
            return saveError(result.error);
        }
        QJsonObject body = articleJson(result.article);
        body.insert(QStringLiteral("updatedExisting"), result.updatedExisting);
        return Response{200, body};
    });
}

bool SaveApi::isAllowedOrigin(const QByteArray &origin)
{
    if (origin.isEmpty()) {
        // not a browser request (curl, a command line tool, ...)
        return true;
    }
    static const QByteArrayList kExtensionSchemes{"moz-extension://", "chrome-extension://", "safari-web-extension://"};
    return std::any_of(kExtensionSchemes.begin(), kExtensionSchemes.end(), [&origin](const QByteArray &scheme) {
        return origin.startsWith(scheme);
    });
}

bool SaveApi::isAllowedHost(const QByteArray &host, quint16 port)
{
    const QByteArray suffix = ':' + QByteArray::number(port);
    static const QByteArrayList kLoopbackNames{"127.0.0.1", "localhost", "[::1]"};
    return std::any_of(kLoopbackNames.begin(), kLoopbackNames.end(), [&](const QByteArray &name) {
        return host.compare(name + suffix, Qt::CaseInsensitive) == 0;
    });
}
