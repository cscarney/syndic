/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QObject>

class QString;
class QUrl;

namespace FeedCore
{
class ReadabilityResult;
class Readability : public QObject
{
    Q_OBJECT
public:
    virtual ~Readability() = default;

    /**
     * Download the page at /url/ and extract its readable content.
     */
    virtual ReadabilityResult *fetch(const QUrl &url) = 0;

    /**
     * Extract readable content from an HTML document that has already been
     * retrieved (e.g. the DOM of a page open in a browser). /url/ is used to
     * resolve relative links.
     */
    virtual ReadabilityResult *parse(const QString &html, const QUrl &url) = 0;
};
}
