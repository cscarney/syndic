#include "feeddiscovery.h"
using namespace FeedCore;

namespace
{
enum FeedCandidateScores : int { GuessFeedUrlScore = 0, FeedLikeLinkScore, FeedLikeAnchorScore, ExplicitFeedLinkScore };
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
    return (href.endsWith(".xml") || href.endsWith(".rdf") || href.endsWith(".rss") || href.endsWith("/feed"));
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
    if (rel != "alternate") {
        return;
    }

    QString href = getAttrString(element, "href");
    if (href.isEmpty()) {
        return;
    }

    QString type = getAttrString(element, "type");
    if (type == "application/rss+xml") {
        discovered(ExplicitFeedLinkScore, href);
        return;
    }

    if (!type.isEmpty()) {
        // some non-rss type, ignore
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
        m_bestCandidate = url;
    }
}
