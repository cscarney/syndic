/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NEWITEMNOTIFIER_H
#define NEWITEMNOTIFIER_H

#include <QObject>
#include "feed.h"
namespace FeedCore {
    class Context;
}

class NewItemNotifier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FeedCore::Context *context READ context WRITE setContext NOTIFY contextChanged);
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged);
public:
    explicit NewItemNotifier(QObject *parent = nullptr);
    void setContext(FeedCore::Context *context);
    FeedCore::Context *context() const;
    void setEnabled(bool flag);
    bool enabled() const;

signals:
    void contextChanged();
    void enabledChanged();

private:
    FeedCore::Context *m_context { nullptr };
    FeedCore::Feed *m_feed { nullptr };
    int m_counter { 0 };
    bool m_enabled { false };
    void onStatusChanged();
    void onArticleAdded(const FeedCore::ArticleRef &article);
    void postNotification() const;
};

#endif // NEWITEMNOTIFIER_H
