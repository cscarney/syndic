/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "opmlreader.h"
#include "provisionalfeed.h"
#include <QUrl>

using namespace FeedCore;

namespace
{
enum ElementType { Feed, Category, Other };
}

OpmlReader::OpmlReader(QIODevice *device)
    : xml(device)
{
}

OpmlReader::OpmlReader(QIODevice *device, const QSet<Feed *> &existingFeeds)
    : OpmlReader(device)
{
    for (Feed *feed : existingFeeds) {
        m_existingFeeds[feed->url()] = feed;
    }
}

void OpmlReader::readAll()
{
    QList<ElementType> openElements;
    QStringList categories;
    while (!xml.atEnd()) {
        if (xml.hasError()) {
            return;
        }
        xml.readNext();
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrs{xml.attributes()};
            if (xml.name() == "outline") {
                if (!attrs.hasAttribute("xmlUrl")) {
                    openElements.append(ElementType::Category);
                    categories.append(attrs.value("text").toString());
                } else {
                    openElements.append(ElementType::Feed);
                    QUrl xmlUrl(attrs.value("xmlUrl").toString());
                    if (!xmlUrl.isValid()) {
                        xml.raiseError(tr("Invalid URL in OPML file: %s").arg(xmlUrl.toString()));
                        return;
                    }
                    QString text{attrs.value("text").toString()};
                    QString category = categories.join('/');
                    foundFeed(xmlUrl, text, category);
                }
            } else {
                openElements.append(ElementType::Other);
            }
        } else if (xml.isEndElement()) {
            ElementType elementType = openElements.takeLast();
            if (elementType == ElementType::Category) {
                categories.removeLast();
            }
        }
    }
}

void OpmlReader::foundFeed(const QUrl &xmlUrl, const QString &text, const QString &category)
{
    auto *feed = new ProvisionalFeed(this);
    feed->setUrl(xmlUrl);
    feed->setName(text);
    feed->setCategory(category);
    if (m_existingFeeds.contains(xmlUrl)) {
        feed->setTargetFeed(m_existingFeeds.value(xmlUrl));
        m_updatedFeeds.insert(feed);
    } else {
        m_newFeeds.insert(feed);
    }
}
