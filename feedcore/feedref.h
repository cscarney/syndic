#ifndef FEEDREF_H
#define FEEDREF_H
#include <QSharedPointer>

namespace FeedCore {
class Feed;
// typedef QSharedPointer<Feed> FeedRef;

class FeedRef : public QSharedPointer<Feed> {
    Q_GADGET
public:
    using QSharedPointer<Feed>::QSharedPointer;
    Q_PROPERTY(Feed *feed READ get CONSTANT);
};

}

#endif // FEEDREF_H
