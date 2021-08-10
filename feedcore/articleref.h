#ifndef FEEDCORE_ARTICLEREF_H
#define FEEDCORE_ARTICLEREF_H
#include <QSharedPointer>

namespace FeedCore {
class Article;
typedef QSharedPointer<Article> ArticleRef;
}
#endif // FEEDCORE_ARTICLEREF_H
