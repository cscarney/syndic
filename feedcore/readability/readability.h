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
class Readability : public QObject
{
    Q_OBJECT
public:
    virtual ~Readability() = default;

    virtual void fetch(const QString &url) = 0;

Q_SIGNALS:
    void startedFetching(const QString &url);
    void finishedFetching(const QString &url, QString readable);
    void errorFetching(const QString &url);
};
}

#endif
