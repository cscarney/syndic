/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "urlmatch.h"
#include <QStringList>
#include <QUrlQuery>
#include <algorithm>

using namespace FeedCore;

QString UrlMatch::normalizedHost(const QUrl &url)
{
    QString host = url.host().toLower();
    static const QStringList kIgnoredPrefixes{QStringLiteral("www."), QStringLiteral("m."), QStringLiteral("amp.")};
    for (const QString &prefix : kIgnoredPrefixes) {
        if (host.startsWith(prefix) && host.size() > prefix.size()) {
            host = host.mid(prefix.size());
            break;
        }
    }
    return host;
}

static QStringList normalizedPathSegments(const QUrl &url)
{
    QStringList segments = url.path(QUrl::FullyEncoded).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (!segments.isEmpty()) {
        static const QStringList kIndexPages{QStringLiteral("index.html"), QStringLiteral("index.htm"), QStringLiteral("index.php")};
        if (kIndexPages.contains(segments.last().toLower())) {
            segments.removeLast();
        }
    }
    if (!segments.isEmpty() && segments.last().toLower() == QStringLiteral("amp")) {
        segments.removeLast();
    }
    return segments;
}

static bool isTrackingParameter(const QString &key)
{
    static const QStringList kTrackingParameters{
        QStringLiteral("fbclid"),
        QStringLiteral("gclid"),
        QStringLiteral("dclid"),
        QStringLiteral("msclkid"),
        QStringLiteral("mc_cid"),
        QStringLiteral("mc_eid"),
        QStringLiteral("ref"),
        QStringLiteral("ref_src"),
        QStringLiteral("ocid"),
        QStringLiteral("cmpid"),
        QStringLiteral("smid"),
        QStringLiteral("amp"),
        QStringLiteral("outputType"),
    };
    const QString lower = key.toLower();
    return lower.startsWith(QStringLiteral("utm_")) || kTrackingParameters.contains(key, Qt::CaseInsensitive);
}

QString UrlMatch::normalizedUrlKey(const QUrl &url)
{
    const QString host = normalizedHost(url);
    if (host.isEmpty()) {
        return {};
    }

    QString key = host;
    const int port = url.port();
    if (port > 0 && port != 80 && port != 443) {
        key += QLatin1Char(':') + QString::number(port);
    }

    const QStringList segments = normalizedPathSegments(url);
    for (const QString &segment : segments) {
        key += QLatin1Char('/') + segment;
    }

    QList<QPair<QString, QString>> params;
    const auto items = QUrlQuery(url).queryItems(QUrl::FullyEncoded);
    for (const auto &item : items) {
        if (!isTrackingParameter(item.first)) {
            params << item;
        }
    }
    if (!params.isEmpty()) {
        std::sort(params.begin(), params.end());
        QStringList parts;
        for (const auto &param : std::as_const(params)) {
            parts << param.first + QLatin1Char('=') + param.second;
        }
        key += QLatin1Char('?') + parts.join(QLatin1Char('&'));
    }
    return key;
}

int UrlMatch::siteMatchScore(const QUrl &pageUrl, const QUrl &siteUrl)
{
    const QString pageHost = normalizedHost(pageUrl);
    if (pageHost.isEmpty() || pageHost != normalizedHost(siteUrl)) {
        return -1;
    }

    const QStringList pageSegments = normalizedPathSegments(pageUrl);
    const QStringList siteSegments = normalizedPathSegments(siteUrl);
    if (siteSegments.size() > pageSegments.size()) {
        return -1;
    }
    for (qsizetype i = 0; i < siteSegments.size(); ++i) {
        if (siteSegments[i] != pageSegments[i]) {
            return -1;
        }
    }
    return int(siteSegments.size());
}
