/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FEEDCORE_READABILITY_H
#define FEEDCORE_READABILITY_H

#include <QObject>

class QString;

namespace FeedCore
{
class ReadabilityResult;
class Readability : public QObject
{
    Q_OBJECT
public:
    virtual ~Readability() = default;

    virtual ReadabilityResult *fetch(const QUrl &url) = 0;
};
}

#endif
