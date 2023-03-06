/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "articleref.h"
#include <QObject>

namespace FeedCore
{
class Context;
class Feed;
class AutomationRule;
class AutomationEngine : public QObject
{
    Q_OBJECT
public:
    explicit AutomationEngine(Context *parent);
    void processArticle(const ArticleRef &article);
    const QList<FeedCore::AutomationRule *> &automationRules();
    FeedCore::AutomationRule *addAutomationRule();
    bool removeAutomationRule(FeedCore::AutomationRule *rule);
    static AutomationEngine *fromDefaultConfigFile(Context *parent);

private:
    QList<AutomationRule *> m_rules;
    QSharedPointer<Feed> m_monitorFeed;
    bool loadJson(const QString &path);
};

} // namespace FeedCore
