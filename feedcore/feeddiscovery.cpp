#include "feeddiscovery.h"
#include <QStringList>
using namespace FeedCore;

namespace
{
enum FeedCandidateScores : int { GuessFeedUrlScore = 0, FeedLikeLinkScore, FeedLikeAnchorScore, GenericXMLLinkScore, ExplicitFeedLinkScore };
}
static QUrl slashFeed(const QUrl &url)
{
    QUrl result = url;
    result.setPath("/feed");
    return result;
}

QUrl FeedDiscovery::discoverFeed(const QUrl &url, const QByteArray &html)
{
    FeedDiscovery discovery(url, html);
    discovery.walk();
    return url.resolved(discovery.m_bestCandidate);
}

FeedDiscovery::FeedDiscovery(const QUrl &url, const QByteArray &html)
    : GumboVisitor(html)
    , m_bestCandidate(slashFeed(url))
    , m_bestScore{GuessFeedUrlScore}
{
}

static bool looksLikeFeed(const QString &href)
{
    return (href.endsWith(".xml") || href.endsWith(".rdf") || href.endsWith(".rss") || href.endsWith("/feed") || href.contains("//feeds."));
}

/**
 * rel is a space-separated list of tokens, e.g. rel="alternate home"
 */
static bool hasRelToken(const QString &rel, const QString &token)
{
    const QStringList tokens = rel.simplified().split(' ', Qt::SkipEmptyParts);
    for (const QString &candidate : tokens) {
        if (candidate.compare(token, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Strip any parameters, e.g. type="application/atom+xml; charset=utf-8"
 */
static QString mediaType(const QString &typeAttribute)
{
    return typeAttribute.split(';').first().trimmed();
}

static bool isExplicitFeedType(const QString &type)
{
    static const QStringList feedTypes = {"application/rss+xml", "application/atom+xml", "application/rdf+xml"};
    return feedTypes.contains(type, Qt::CaseInsensitive);
}

static bool isGenericXmlType(const QString &type)
{
    static const QStringList xmlTypes = {"application/xml", "text/xml"};
    return xmlTypes.contains(type, Qt::CaseInsensitive);
}

static QString getAttrString(const GumboElement &element, const char *nameString)
{
    GumboAttribute *attr = gumbo_get_attribute(&element.attributes, nameString);
    if (attr == nullptr) {
        return QString();
    }
    return QString(attr->value);
}

void FeedDiscovery::visitLinkElement(const GumboElement &element)
{
    QString rel = getAttrString(element, "rel");
    if (!hasRelToken(rel, "alternate")) {
        return;
    }

    QString href = getAttrString(element, "href");
    if (href.isEmpty()) {
        return;
    }

    QString type = mediaType(getAttrString(element, "type"));
    if (isExplicitFeedType(type)) {
        discovered(ExplicitFeedLinkScore, href);
        return;
    }

    if (isGenericXmlType(type)) {
        discovered(GenericXMLLinkScore, href);
        return;
    }

    if (!type.isEmpty()) {
        // some non-feed type; ignore
        return;
    }

    if (looksLikeFeed(href)) {
        discovered(FeedLikeLinkScore, href);
        return;
    }
}

void FeedDiscovery::visitAnchorElement(const GumboElement &element)
{
    QString href = getAttrString(element, "href");
    if (looksLikeFeed(href)) {
        discovered(FeedLikeAnchorScore, href);
    }
}

void FeedDiscovery::visitElementOpen(GumboNode *node)
{
    GumboElement &element = node->v.element;
    switch (element.tag) {
    case GUMBO_TAG_LINK:
        visitLinkElement(element);
        break;

    case GUMBO_TAG_A:
        visitAnchorElement(element);

    default:
        break;
    }
}

void FeedDiscovery::discovered(int score, const QUrl &url)
{
    if (score > m_bestScore) {
        m_bestScore = score;
        m_bestCandidate = url;
    }
}
