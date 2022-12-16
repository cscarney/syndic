#pragma once

#include <article.h>

namespace TTRSS
{
class StorageImpl;
class FeedImpl;

class ArticleImpl : public FeedCore::Article
{
    Q_OBJECT
public:
    ArticleImpl(int articleId, FeedImpl *feed, StorageImpl *parent);
    void requestContent() override;
    void updateFromJson(const QJsonObject &apiObject);

private:
    int m_articleId;
};

}
