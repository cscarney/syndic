#pragma once
#include "article.h"

constexpr const char *kMockArticleContent = "mock article content";

class MockArticle : public FeedCore::Article
{
public:
    MockArticle(FeedCore::Feed *feed, QObject *parent = nullptr)
        : FeedCore::Article(feed, parent)
    {
    }

    void requestContent() override
    {
        emit gotContent(kMockArticleContent);
    }
};
