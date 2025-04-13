/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "articleref.h"
#include <QFuture>
#include <QObject>
#include <QQueue>

namespace FeedCore
{
class Context;
class Feed;
class AbstractAutomationRule;
class AutomationEngine : public QObject
{
    Q_OBJECT
public:
    explicit AutomationEngine(Context *parent);
    void processArticle(const ArticleRef &article);
    const QList<FeedCore::AbstractAutomationRule *> &automationRules();
    FeedCore::AbstractAutomationRule *addAutomationRule();
    void addAutomationRule(FeedCore::AbstractAutomationRule *rule);
    bool removeAutomationRule(FeedCore::AbstractAutomationRule *rule);
    static AutomationEngine *fromDefaultConfigFile(Context *parent);
    bool active();

private:
    struct PendingAction {
        AbstractAutomationRule *rule;
        ArticleRef article;
    };

    QList<AbstractAutomationRule *> m_rules;
    QSharedPointer<Feed> m_monitorFeed;
    QQueue<PendingAction> m_actionQueue;
    int m_activeActions = 0;

    bool loadJson(const QString &path);
    void beginNextAction();
    void processPendingActions();
    void onActionFinished();
    static int maxConcurrentActions();
};

} // namespace FeedCore
