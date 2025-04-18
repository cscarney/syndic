/**
 * ArticleLinkExtractor.cpp
 * Implementation of the ArticleLinkExtractor class
 */

#include "articlelinkextractor.h"
#include <QDomElement>
#include <Syndication/Item>

namespace FeedCore
{

class ArticleLinkItem : public Syndication::Item
{
public:
    ArticleLink m_link;
    explicit ArticleLinkItem(const ArticleLink &link)
        : m_link(link)
    {
    }

    Syndication::SpecificItemPtr specificItem() const override
    {
        return Syndication::SpecificItemPtr(nullptr);
    }

    QString title() const override
    {
        return m_link.linkText;
    }
    QString link() const override
    {
        return m_link.url.toString();
    }

    QString description() const override
    {
        return QString();
    }

    QString content() const override
    {
        return QString();
    }

    time_t datePublished() const override
    {
        if (m_link.hasDate()) {
            return m_link.date.toSecsSinceEpoch();
        }
        return 0;
    }
    time_t dateUpdated() const override
    {
        return datePublished();
    }

    QString id() const override
    {
        return m_link.url.toString();
    }

    QList<Syndication::PersonPtr> authors() const override
    {
        return QList<Syndication::PersonPtr>();
    }

    QString language() const override
    {
        // return system language by default
        return QLocale::system().name();
    }

    QList<Syndication::EnclosurePtr> enclosures() const override
    {
        return QList<Syndication::EnclosurePtr>();
    }

    QList<Syndication::CategoryPtr> categories() const override
    {
        return QList<Syndication::CategoryPtr>();
    }

    int commentsCount() const override
    {
        return 0;
    }

    QString commentsLink() const override
    {
        return QString();
    }

    QString commentsFeed() const override
    {
        return QString();
    }

    QString commentPostUri() const override
    {
        return QString();
    }

    QMultiMap<QString, QDomElement> additionalProperties() const override
    {
        return QMultiMap<QString, QDomElement>();
    }
};

class ArticleLinkFeed : public Syndication::Feed
{
public:
    QString m_title;
    QString m_url;
    QList<ArticleLink> m_articleLinks;
    explicit ArticleLinkFeed(const QString &title, const QString &url, const QList<ArticleLink> &articleLinks)
        : m_title(title)
        , m_url(url)
        , m_articleLinks(articleLinks)
    {
    }

    Syndication::SpecificDocumentPtr specificDocument() const override
    {
        return Syndication::SpecificDocumentPtr(nullptr);
    }

    QList<Syndication::ItemPtr> items() const override
    {
        QList<Syndication::ItemPtr> itemList;
        for (const ArticleLink &link : m_articleLinks) {
            itemList.append(Syndication::ItemPtr(new ArticleLinkItem(link)));
        }
        return itemList;
    }
    QList<Syndication::CategoryPtr> categories() const override
    {
        return QList<Syndication::CategoryPtr>();
    }

    QString title() const override
    {
        return m_title;
    }
    QString link() const override
    {
        return m_url;
    }
    QString description() const override
    {
        return QString();
    }
    Syndication::ImagePtr image() const override
    {
        return Syndication::ImagePtr();
    }
    Syndication::ImagePtr icon() const override
    {
        return Syndication::ImagePtr();
    }
    QList<Syndication::PersonPtr> authors() const override
    {
        return QList<Syndication::PersonPtr>();
    }
    QString language() const override
    {
        // return system language by default
        return QLocale::system().name();
    }
    QString copyright() const override
    {
        return QString();
    }
    QMultiMap<QString, QDomElement> additionalProperties() const override
    {
        return QMultiMap<QString, QDomElement>();
    }
};

enum { EmptyLinkScore = -1, DefaultLinkScore = 0, ExcludeLinkScore = 1, SubstanitalContentScore = 2, IncludeLinkScore = 3, DateLinkScore = 4 };

ArticleLinkExtractor::ArticleLinkExtractor(const QString &input, const QUrl &baseUrl)
    : GumboVisitor(input)
    , m_inAnchor(false)
    , m_inTitle(false)
    , m_baseUrl(baseUrl)
    , m_highestScore(0)
    , m_datePattern(R"(/(\d{4})/(\d{2})/(\d{2})/)")
{
}

ArticleLinkExtractor::ArticleLinkExtractor(const QByteArray &utf8data, const QUrl &baseUrl)
    : GumboVisitor(utf8data)
    , m_inAnchor(false)
    , m_inTitle(false)
    , m_baseUrl(baseUrl)
    , m_datePattern(R"(/(\d{4})/(\d{2})/(\d{2})/)")
{
}

void ArticleLinkExtractor::setBaseUrl(const QUrl &baseUrl)
{
    m_baseUrl = baseUrl;
}

ArticleLinkExtractor::~ArticleLinkExtractor()
{
}

QList<ArticleLink> ArticleLinkExtractor::articleLinks() const
{
    return m_articleLinks;
}

Syndication::FeedPtr ArticleLinkExtractor::articleLinksFeed() const
{
    // Create a feed with the extracted article links
    QString title = !m_pageTitle.isEmpty() ? m_pageTitle : "Extracted Article Links";
    QString url = m_baseUrl.isValid() ? m_baseUrl.toString() : "http://example.com";
    Syndication::FeedPtr feed(new ArticleLinkFeed(title, url, m_articleLinks));
    return feed;
}

void ArticleLinkExtractor::visitElementOpen(GumboNode *node)
{
    if (node->v.element.tag == GUMBO_TAG_A) {
        m_inAnchor = true;
        m_currentLinkText.clear();

        // Extract href attribute
        GumboAttribute *href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            m_currentHref = QString::fromUtf8(href->value);
        } else {
            m_currentHref.clear();
        }
    } else if (node->v.element.tag == GUMBO_TAG_TITLE) {
        m_inTitle = true;
        m_pageTitle.clear();
    }
}

void ArticleLinkExtractor::visitText(GumboNode *node)
{
    if (m_inAnchor) {
        // Append text content to current link text
        m_currentLinkText += QString::fromUtf8(node->v.text.text);
    } else if (m_inTitle) {
        // Capture the page title
        m_pageTitle += QString::fromUtf8(node->v.text.text);
    }
}

void ArticleLinkExtractor::visitElementClose(GumboNode *node)
{
    if (node->v.element.tag == GUMBO_TAG_A && !m_currentHref.isEmpty()) {
        auto score = articleLinkScore(m_currentHref, m_currentLinkText);
        if (score > m_highestScore) {
            m_highestScore = score;
            m_articleLinks.clear();
        }
        if (score == m_highestScore) {
            ArticleLink link;
            QUrl url(m_currentHref);
            if (m_baseUrl.isValid() && url.isRelative()) {
                url = m_baseUrl.resolved(url);
            }
            link.url = url;
            link.linkText = m_currentLinkText.trimmed();

            if (score == DateLinkScore) {
                QRegularExpressionMatch match = m_datePattern.match(m_currentHref);
                if (match.hasMatch()) {
                    int year = match.captured(1).toInt();
                    int month = match.captured(2).toInt();
                    int day = match.captured(3).toInt();

                    link.date = QDateTime(QDate(year, month, day), QTime(0, 0));
                }
            }

            m_articleLinks.append(link);
        }

        m_inAnchor = false;
    } else if (node->v.element.tag == GUMBO_TAG_TITLE) {
        m_inTitle = false;
    }
}

int ArticleLinkExtractor::articleLinkScore(const QString &url, const QString &linkText)
{
    // Skip empty links or links without text
    if (url.isEmpty() || linkText.trimmed().isEmpty()) {
        qDebug() << url << "excluded due to empty link or text";
        return EmptyLinkScore;
    }

    // Skip navigation links, social media links, etc.
    static const QStringList excludePatterns = {"#",
                                                "javascript:",
                                                "mailto:",
                                                "tel:",
                                                "/tag/",
                                                "/category/",
                                                "/author/",
                                                "/page/",
                                                "facebook.com",
                                                "twitter.com",
                                                "instagram.com",
                                                "youtube.com",
                                                "/login",
                                                "/register",
                                                "/subscribe",
                                                "/about",
                                                "/contact"};

    for (const auto &pattern : excludePatterns) {
        if (url.contains(pattern, Qt::CaseInsensitive)) {
            return ExcludeLinkScore;
        }
    }

    if (m_datePattern.match(url).hasMatch()) {
        return DateLinkScore;
    }

    static const QStringList articlePatterns = {"/article/", "/story/", "/blog/", "/post/", "/entry/", ".html", ".php"};

    for (const auto &pattern : articlePatterns) {
        if (url.contains(pattern, Qt::CaseInsensitive)) {
            return IncludeLinkScore;
        }
    }

    // Favor links with substantial text (likely a headline)
    if (linkText.length() > 20 && linkText.length() < 200) {
        return SubstanitalContentScore;
    }

    // Default score for anything else
    return DefaultLinkScore;
}

} // namespace FeedCore
