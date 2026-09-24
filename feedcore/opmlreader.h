/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QObject>
#include <QSet>
#include <QXmlStreamReader>

namespace FeedCore
{
class Feed;
class Subscription;
class ProvisionalSubscription;
class OpmlReader : public QObject
{
    Q_OBJECT
public:
    explicit OpmlReader(QIODevice *device);

    /**
     * \warning All of the feed pointers in existingFeeds must remain valid for the life of the reader
     */
    OpmlReader(QIODevice *device, const QSet<Subscription *> &existingFeeds);

    void readAll();
    bool hasError() const
    {
        return xml.hasError();
    }
    QString errorString() const
    {
        return xml.errorString();
    }

    const QSet<ProvisionalSubscription *> &newFeeds()
    {
        return m_newFeeds;
    }
    const QSet<ProvisionalSubscription *> &updatedFeeds()
    {
        return m_updatedFeeds;
    }

private:
    QXmlStreamReader xml;
    QHash<QUrl, Subscription *> m_existingFeeds;
    QSet<ProvisionalSubscription *> m_newFeeds;
    QSet<ProvisionalSubscription *> m_updatedFeeds;
    void foundFeed(const QUrl &xmlUrl, const QString &text, const QString &category);
};
}
