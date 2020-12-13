#ifndef FEEDREFWRAPPER_H
#define FEEDREFWRAPPER_H
#include <QQmlEngine>
#include "feedref.h"
using namespace FeedCore;

class FeedRefWrapper {
    Q_GADGET
public:
    inline FeedRefWrapper() = default;

    FeedRefWrapper(const FeedRef &ref);

    inline Feed *feed() const { return p.get(); };
    Q_PROPERTY(Feed *feed READ feed CONSTANT);

    inline FeedRef ref() const { return p; };
    Q_PROPERTY(FeedRef ref READ ref CONSTANT);

    inline operator FeedRef() const { return p; }
private:
    FeedRef p;
};

Q_DECLARE_METATYPE(FeedRefWrapper);

#endif // FEEDREFWRAPPER_H
