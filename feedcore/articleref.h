#ifndef ARTICLEREF_H
#define ARTICLEREF_H
#include <QSharedPointer>

namespace FeedCore {
class Article;
typedef QSharedPointer<Article> ArticleRef;
}
#endif // ARTICLEREF_H
