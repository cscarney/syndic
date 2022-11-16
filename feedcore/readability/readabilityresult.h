/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FEEDCORE_READABILITYRESULT_H
#define FEEDCORE_READABILITYRESULT_H
#include <QObject>

namespace FeedCore
{
class ReadabilityResult : public QObject
{
    Q_OBJECT

protected:
    using QObject::QObject;

signals:
    void finished(const QString &content);
    void error();
};
}

#endif // FEEDCORE_READABILITYRESULT_H
