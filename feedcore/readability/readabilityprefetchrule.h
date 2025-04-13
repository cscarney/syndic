/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "automation/abstractautomationrule.h"
#include <QObject>

namespace FeedCore
{
class Readability;
class ReadabilityResult;

class ReadabilityPrefetchRule : public AbstractAutomationRule
{
    Q_OBJECT

public:
    explicit ReadabilityPrefetchRule(Readability *readability, QObject *parent = nullptr);
    bool matches(const ArticleRef &article) override;
    QFuture<void> beginPerformAction(const ArticleRef &article) override;

private:
    Readability *m_readability;
};

} // namespace FeedCore
