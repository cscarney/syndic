/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QString>
#include <QUrl>

namespace FeedCore::UrlMatch
{
/**
 * Reduce a URL to a key that is shared by "equivalent" URLs.
 *
 * The key ignores the scheme, common host prefixes (www., m., amp.),
 * default ports, fragments, trailing slashes and index pages, AMP
 * suffixes, and well-known tracking parameters. The remaining query
 * parameters are sorted.
 *
 * Returns an empty string for URLs that have no host.
 */
QString normalizedUrlKey(const QUrl &url);

/**
 * The lowercased host of /url/, without common prefixes (www., m., amp.).
 */
QString normalizedHost(const QUrl &url);

/**
 * Score how well a page URL fits under a feed's site link.
 *
 * Both URLs must have the same normalized host, and the path of
 * /siteUrl/ must be a prefix of the path of /pageUrl/ (compared by path
 * segment). The score is the number of matching path segments, so a more
 * specific site link scores higher. Returns -1 when the page does not
 * belong to the site.
 */
int siteMatchScore(const QUrl &pageUrl, const QUrl &siteUrl);
}
