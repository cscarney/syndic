/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "articleref.h"
#include <QObject>

namespace FeedCore
{
class AutomationEngine;
class AutomationRule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FeedCore::AutomationRule::MatchField matchField READ matchField WRITE setMatchField NOTIFY matchFieldChanged)
    Q_PROPERTY(QString matchValue READ matchValue WRITE setMatchValue NOTIFY matchValueChanged)
    Q_PROPERTY(bool shouldMarkStarred READ shouldMarkStarred WRITE setShouldMarkStarred NOTIFY shouldMarkStarredChanged)
    Q_PROPERTY(bool shouldMarkRead READ shouldMarkRead WRITE setShouldMarkRead NOTIFY shouldMarkReadChanged)
    Q_PROPERTY(bool shouldRunShellScript READ shouldRunShellScript WRITE setShouldRunShellScript NOTIFY shouldRunShellScriptChanged)
    Q_PROPERTY(QString shellScript READ shellScript WRITE setShellScript NOTIFY shellScriptChanged)
public:
    enum MatchField { MatchEverything = 0, MatchFeed, MatchAuthor, MatchTitle };
    Q_ENUM(MatchField);

    void apply(const ArticleRef &article);

    FeedCore::AutomationRule::MatchField matchField() const;
    void setMatchField(FeedCore::AutomationRule::MatchField newMatchField);

    const QString &matchValue() const;
    void setMatchValue(const QString &newMatchValue);

    bool shouldMarkStarred() const;
    void setShouldMarkStarred(bool newShouldMarkStarred);

    bool shouldMarkRead() const;
    void setShouldMarkRead(bool newShouldMarkRead);

    bool shouldRunShellScript() const;
    void setShouldRunShellScript(bool newShouldRunShellScript);

    const QString &shellScript() const;
    void setShellScript(const QString &newShellScript);

signals:
    void postNotification(const FeedCore::ArticleRef &article);

    void matchFieldChanged();
    void matchValueChanged();

    void shouldMarkStarredChanged();

    void shouldMarkReadChanged();

    void shouldPostNotificationChanged();

    void shouldRunShellScriptChanged();

    void shellScriptChanged();

private:
    FeedCore::AutomationRule::MatchField m_matchField{MatchEverything};
    QString m_matchValue;
    bool m_shouldMarkStarred{false};
    bool m_shouldMarkRead{false};
    bool m_shouldRunShellScript{false};
    QString m_shellScript;

    explicit AutomationRule(QObject *parent = nullptr);
    explicit AutomationRule(const QJsonObject &obj, QObject *parent);
    bool matches(const ArticleRef &article);
    void performAction(const ArticleRef &article);
    friend class FeedCore::AutomationEngine;
};
}
