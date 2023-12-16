#include "article.h"
#include "articlesummary.h"
#include "mockfeed.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>
using namespace FeedCore;

class MockArticle : public FeedCore::Article
{
    QString m_articleText;

public:
    MockArticle(Feed *feed, QString text)
        : FeedCore::Article(feed, nullptr)
        , m_articleText(text)
    {
    }

    void requestContent() override
    {
        emit gotContent(m_articleText);
    }
};

class testArticleSummary : public QObject
{
    Q_OBJECT
    MockFeed m_mockFeed;
private slots:
    void initTestCase()
    {
    }

    void init()
    {
    }

    void cleanup()
    {
    }

    void testSelectsFirstParagraph()
    {
        constexpr const char *kArticleText = "<p>This is the first paragraph.</p><p>This is the second paragraph.</p>";
        auto article = ArticleRef(new MockArticle(&m_mockFeed, kArticleText));
        ArticleSummary summary;
        summary.setArticle(article);
        QVERIFY(QTest::qWaitFor([&]() {
            return summary.finished();
        }));
        QCOMPARE(summary.firstParagraph(), QString("This is the first paragraph."));
    }

    void testSelectedParagraphCollapsesWhitespace()
    {
        constexpr const char *kParagraphWithWhitespace = "<p>This is the first\n\nparagraph.</p><p>Second paragraph</p>";
        auto article = ArticleRef(new MockArticle(&m_mockFeed, kParagraphWithWhitespace));
        ArticleSummary summary;
        summary.setArticle(article);
        QVERIFY(QTest::qWaitFor([&]() {
            return summary.finished();
        }));
        QCOMPARE(summary.firstParagraph(), QString("This is the first paragraph."));
    }

    void testUsesFreeTextWhenNoParagraphs()
    {
        constexpr const char *kArticleText = "This is the first paragraph.";
        auto article = ArticleRef(new MockArticle(&m_mockFeed, kArticleText));
        ArticleSummary summary;
        summary.setArticle(article);
        QVERIFY(QTest::qWaitFor([&]() {
            return summary.finished();
        }));
        QCOMPARE(summary.firstParagraph(), QString("This is the first paragraph."));
    }

    void testCollapsesWhitespaceWhenNoParagraphs()
    {
        constexpr const char *kArticleText = "This is the first\n\nparagraph.";
        auto article = ArticleRef(new MockArticle(&m_mockFeed, kArticleText));
        ArticleSummary summary;
        summary.setArticle(article);
        QVERIFY(QTest::qWaitFor([&]() {
            return summary.finished();
        }));
        QCOMPARE(summary.firstParagraph(), QString("This is the first paragraph."));
    }

    void textExcludesVeryShortParagraphs()
    {
        constexpr const char *kArticleText = "<p>First paragraph.</p><p>This is the second paragraph.</p><p>Third paragraph.</p>";
        auto article = ArticleRef(new MockArticle(&m_mockFeed, kArticleText));
        ArticleSummary summary;
        summary.setArticle(article);
        QVERIFY(QTest::qWaitFor([&]() {
            return summary.finished();
        }));
        QCOMPARE(summary.firstParagraph(), QString("This is the second paragraph."));
    }
};

QTEST_MAIN(testArticleSummary)
#include "tst_articlesummary.moc"
