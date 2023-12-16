#pragma once

#include "articleref.h"
#include <QObject>
#include <QUrl>

namespace FeedCore
{

class ArticleSummary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ArticleRef article READ article WRITE setArticle NOTIFY articleChanged FINAL)
    Q_PROPERTY(QString firstParagraph READ firstParagraph NOTIFY firstParagraphChanged FINAL)
    Q_PROPERTY(QUrl firstImage READ firstImage NOTIFY firstImageChanged FINAL)
    Q_PROPERTY(bool finished READ finished NOTIFY finishedChanged FINAL)
public:
    explicit ArticleSummary(QObject *parent = nullptr);
    ~ArticleSummary() override;

    QString firstParagraph() const;
    QUrl firstImage() const;
    bool finished() const;

    ArticleRef article() const;
    void setArticle(ArticleRef newArticle);

signals:
    void firstParagraphChanged();
    void firstImageChanged();
    void finishedChanged();
    void articleChanged();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    void setFirstParagraph(const QString &firstParagraph);
    void setFirstImage(const QUrl &firstImage);
    void setFinished(bool finished);

    class Summarizer;
};

} // namespace FeedCore
