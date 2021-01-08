#ifndef FEEDREF_H
#define FEEDREF_H
#include <QSharedPointer>

namespace FeedCore {
class Feed;
typedef QSharedPointer<Feed> FeedRef;
}
#endif // FEEDREF_H
