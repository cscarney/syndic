 #ifndef FEEDCORE_PREVIEW_ARTICLEIMPL_H
#define FEEDCORE_PREVIEW_ARTICLEIMPL_H
#include <Syndication/Item>
#include "article.h"
namespace FeedCore::Preview {

/**
 * A minimal implementation of Article, backed directly by a Syndication::Item
 *
 * This is used by ProvisionalFeed to provide feed previews.
 */
class ArticleImpl : public Article {
    Q_OBJECT
public:
    explicit ArticleImpl(Syndication::ItemPtr item, Feed *feed, QObject *parent=nullptr);
    void requestContent() final;

    /**
     * setRead is a no-op because preview articles do not support state tracking
     */
    void setRead(bool isRead) final {};

    /**
     * setStarred is a no-op because preview articles do not support state tracking
     */
    void setStarred(bool isStarred) final {};
private:
    Syndication::ItemPtr m_item;
};
}
#endif // FEEDCORE_PREVIEW_ARTICLEIMPL_H
