#ifndef FEEDREF_H
#define FEEDREF_H
#include <QSharedPointer>

namespace FeedCore {
class Feed;

class FeedRef : public QSharedPointer<Feed> {
    Q_GADGET
public:
    using QSharedPointer<Feed>::QSharedPointer;
};
}
#endif // FEEDREF_H
