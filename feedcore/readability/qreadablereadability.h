/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FEEDCORE_QREADABLEREADABILITY_H
#define FEEDCORE_QREADABLEREADABILITY_H

#include "qprocessreadability.h"

namespace FeedCore
{
class QReadableReadability : public QProcessReadability
{
    Q_OBJECT
public:
    QReadableReadability();
    virtual ~QReadableReadability() = default;
    static bool isSupported();

protected:
    QString program() const override;
    QStringList arguments(const QString &url) const override;
};
}

#endif
