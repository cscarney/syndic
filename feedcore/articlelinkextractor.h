/**
 * ArticleLinkExtractor.h
 * A GumboVisitor subclass that extracts article links from web pages
 */

#pragma once
#include "gumbovisitor.h"
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QUrl>
#include <Syndication/Feed>

namespace FeedCore
{

/**
 * Structure to hold article link information
 */
struct ArticleLink {
    QUrl url;
    QString linkText;
    QDateTime date; // will be valid only if date was extracted from URL

    bool hasDate() const
    {
        return date.isValid();
    }
};

/**
 * Extracts article links from HTML using GumboVisitor
 */
class ArticleLinkExtractor : public GumboVisitor
{
public:
    explicit ArticleLinkExtractor(const QString &input, const QUrl &baseUrl = QUrl());
    explicit ArticleLinkExtractor(const QByteArray &utf8data, const QUrl &baseUrl = QUrl());
    virtual ~ArticleLinkExtractor();

    void setBaseUrl(const QUrl &baseUrl);

    /**
     * Returns the list of article links extracted from the HTML
     */
    QList<ArticleLink> articleLinks() const;

    /**
     * returns a Syndication::Feed containing the article links
     */
    Syndication::FeedPtr articleLinksFeed() const;

private:
    bool m_inAnchor;
    bool m_inTitle;
    QString m_pageTitle;
    QString m_currentLinkText;
    QString m_currentHref;
    QList<ArticleLink> m_articleLinks;
    QHash<QUrl, qsizetype> m_urlIndices;
    QUrl m_baseUrl;
    int m_highestScore;

    // Regular expression to extract date from URL pattern /YYYY/MM/DD/
    const QRegularExpression m_datePattern;

    void visitElementOpen(GumboNode *node) override;
    void visitText(GumboNode *node) override;
    void visitElementClose(GumboNode *node) override;

    /**
     * Determines if a link is likely to be an article link based on
     * URL and link text heuristics
     */
    int articleLinkScore(const QString &url, const QString &linkText);
};

} // namespace FeedCore
