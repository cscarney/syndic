#include "feedrefwrapper.h"
#include "feed.h"

FeedRefWrapper::FeedRefWrapper(const FeedCore::FeedRef &ref) :
    p(ref)
{
    auto *f = p.get();
    if (f) {
        QQmlEngine::setObjectOwnership(f,QQmlEngine::CppOwnership);
    }
};
