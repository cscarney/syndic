#ifndef QMLFEEDREF_H
#define QMLFEEDREF_H
#include <QQmlEngine>
#include "feedref.h"
#include "feed.h"

class QmlFeedRef : public FeedCore::FeedRef {
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

    Q_PROPERTY(FeedCore::Feed *feed READ get CONSTANT);
};

Q_DECLARE_METATYPE(QmlFeedRef);

#endif // QMLFEEDREF_H
