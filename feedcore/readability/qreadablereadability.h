/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FEEDCORE_QREADABLEREADABILITY_H
#define FEEDCORE_QREADABLEREADABILITY_H

#include "readability.h"

namespace QReadable
{
class Readable;
}

namespace FeedCore
{

class QReadableReadability : public Readability
{
public:
    QReadableReadability();
    virtual ~QReadableReadability() = default;
    void fetch(const QString &url) override;

private:
    QReadable::Readable *m_readable{nullptr};
};
}

#endif
