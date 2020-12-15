#ifndef FEEDREFWRAPPER_H
#define FEEDREFWRAPPER_H
#include <QQmlEngine>
#include "feedref.h"
#include "feed.h"
using namespace FeedCore;

class QmlFeedRef : public FeedRef {
    Q_GADGET
public:
    inline QmlFeedRef() = default;
    inline QmlFeedRef(const FeedCore::FeedRef &ref) :
        FeedRef(ref)
    {
        auto *f = get();
        if (f) {
            QQmlEngine::setObjectOwnership(f,QQmlEngine::CppOwnership);
        }
    };

    Q_PROPERTY(Feed *feed READ get CONSTANT);
};

Q_DECLARE_METATYPE(QmlFeedRef);

#endif // FEEDREFWRAPPER_H
