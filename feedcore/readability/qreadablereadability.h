/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

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
    virtual ~QReadableReadability();
    ReadabilityResult *fetch(const QUrl &url) override;

private:
    class Worker;
    class Result;
    QThread *m_thread{nullptr};
    Worker *m_worker{nullptr};
    void parse(const QString &rawHtml, const QUrl &url, Result *result);
};
}
