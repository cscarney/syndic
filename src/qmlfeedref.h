#ifndef QMLFEEDREF_H
#define QMLFEEDREF_H
#include "qmlref.h"
#include "feed.h"

class QmlFeedRef : public QmlRef<FeedCore::Feed> {
    Q_GADGET
    Q_PROPERTY(FeedCore::Feed *feed READ get CONSTANT);
    using QmlRef::QmlRef;
};
Q_DECLARE_METATYPE(QmlFeedRef);
#endif // QMLFEEDREF_H
