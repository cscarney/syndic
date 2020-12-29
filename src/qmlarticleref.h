#ifndef QMLARTICLEREF_H
#define QMLARTICLEREF_H
#include <QQmlEngine>
#include "articleref.h"
#include "article.h"

class QmlArticleRef : public FeedCore::ArticleRef {
    Q_GADGET
public:
    inline QmlArticleRef() = default;
    inline QmlArticleRef(const FeedCore::ArticleRef &ref) :
        ArticleRef(ref)
    {
        auto *a = get();
        if (a) {
            QQmlEngine::setObjectOwnership(a,QQmlEngine::CppOwnership);
        }
    };
    Q_PROPERTY(FeedCore::Article *article READ get CONSTANT);
};
Q_DECLARE_METATYPE(QmlArticleRef);
#endif // QMLARTICLEREF_H
