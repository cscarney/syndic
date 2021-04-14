 #ifndef FEEDCORE_PREVIEW_ARTICLEIMPL_H
#define FEEDCORE_PREVIEW_ARTICLEIMPL_H
#include <Syndication/Item>
#include "article.h"
namespace FeedCore::Preview {
class ArticleImpl : public Article {
    Q_OBJECT
public:
    explicit ArticleImpl(Syndication::ItemPtr item, Feed *feed, QObject *parent=nullptr);
    void requestContent() final;
    void setRead(bool isRead) final {};
    void setStarred(bool isStarred) final {};
private:
    Syndication::ItemPtr m_item;
};
}
#endif // FEEDCORE_PREVIEW_ARTICLEIMPL_H
