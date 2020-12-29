#ifndef ARTICLEREF_H
#define ARTICLEREF_H
#include <QSharedPointer>

namespace FeedCore {
class Article;

class ArticleRef : public QSharedPointer<Article> {
    Q_GADGET
public:
    using QSharedPointer<Article>::QSharedPointer;
};
}
#endif // ARTICLEREF_H
