#include "articlesummary.h"
#include "article.h"
#include "gumbovisitor.h"
#include <QMutex>
#include <QRegularExpression>
#include <QThreadPool>
#include <QUrl>
#include <utility>
using namespace FeedCore;

struct ArticleSummary::PrivData {
    ArticleRef article;
    QString firstParagraph;
    QUrl firstImage;
    bool finished = false;
    QFuture<void> future;
};

// Runs in the thread pool and emits signals when it finds the first paragraph and first image.
class ArticleSummary::Summarizer : public GumboVisitor
{
public:
    explicit Summarizer(const QString &content)
        : GumboVisitor{content}
    {
    }

    struct Result {
        ArticleRef sourceArticle;
        QString paragraph;
        QUrl image;
    };

    Result result() const
    {
        return m_result;
    }

private:
    Result m_result;
    QString m_workingParagraph;
    int m_blacklistedTags = 0;

    void visitElementOpen(GumboNode *node) override
    {
        GumboElement &el = node->v.element;
        switch (el.tag) {
        case GUMBO_TAG_IMG:
            if (!m_result.image.isValid()) {
                GumboAttribute *src = gumbo_get_attribute(&el.attributes, "src");
                if (src) {
                    QUrl url{src->value};
                    if (url.isValid()) {
                        m_result.image = url;
                    }
                }
            }
            break;

        case GUMBO_TAG_P:
            if (m_result.paragraph.isEmpty()) {
                m_workingParagraph.clear();
            }
            break;

        case GUMBO_TAG_FIGURE:
            m_blacklistedTags++;
            break;

        default:
            break;
        }
    }

    void visitText(GumboNode *node) override
    {
        if (m_result.paragraph.isEmpty() && m_blacklistedTags == 0) {
            QString text = node->v.text.text;
            m_workingParagraph.append(text);
        }
    }

    static constexpr const int kShortestAcceptableParagraph = 5;

    void acceptParagraph()
    {
        if (m_result.paragraph.isEmpty()) {
            QString simplified = m_workingParagraph.simplified();
            qsizetype wordCount = simplified.count(' ') + 1;
            if (wordCount >= kShortestAcceptableParagraph) {
                m_result.paragraph = simplified;
            }
        }
    }

    void visitElementClose(GumboNode *node) override
    {
        GumboElement &el = node->v.element;
        switch (el.tag) {
        case GUMBO_TAG_P:
            if (m_result.paragraph.isEmpty()) {
                acceptParagraph();
            }
            break;

        case GUMBO_TAG_FIGURE:
            m_blacklistedTags--;

        default:
            break;
        }
    }

    void finished() override
    {
        // If we didn't find a paragraph, use the free text.
        if (m_result.paragraph.isEmpty()) {
            acceptParagraph();
        }
    }
};

ArticleSummary::ArticleSummary(QObject *parent)
    : QObject{parent}
    , d{std::make_unique<PrivData>()}
{
}

ArticleSummary::~ArticleSummary() = default;

QString ArticleSummary::firstParagraph() const
{
    return d->firstParagraph;
}

QUrl ArticleSummary::firstImage() const
{
    return d->firstImage;
}

bool ArticleSummary::finished() const
{
    return d->finished;
}

void ArticleSummary::setFirstParagraph(const QString &firstParagraph)
{
    if (d->firstParagraph == firstParagraph) {
        return;
    }

    d->firstParagraph = firstParagraph;
    emit firstParagraphChanged();
}

void ArticleSummary::setFirstImage(const QUrl &firstImage)
{
    if (d->firstImage == firstImage) {
        return;
    }

    d->firstImage = firstImage;
    emit firstImageChanged();
}

void ArticleSummary::setFinished(bool finished)
{
    if (d->finished == finished) {
        return;
    }
    d->finished = finished;
    emit finishedChanged();
}

ArticleRef ArticleSummary::article() const
{
    return d->article;
}

void ArticleSummary::setArticle(const ArticleRef &newArticle)
{
    if (d->article == newArticle) {
        return;
    }

    d->article = newArticle;
    emit articleChanged();
    d->future.cancel();

    auto fContent = QtFuture::connect(newArticle.get(), &Article::gotContent);
    newArticle->requestContent();
    setFinished(false);

    d->future = fContent
                    .then(QtFuture::Launch::Async,
                          [](std::tuple<QString, Article::ContentType> getContentResult) {
                              auto [content, contentType] = std::move(getContentResult);
                              Summarizer summarizer{content};
                              summarizer.walk();
                              return summarizer.result();
                          })
                    .then(this, [target = QPointer(this)](const Summarizer::Result &summarizerResult) {
                        if (!target) {
                            return;
                        }
                        target->setFirstParagraph(summarizerResult.paragraph);
                        target->setFirstImage(summarizerResult.image);
                        target->setFinished(true);
                    });
}
