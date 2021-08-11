/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QMLARTICLEREF_H
#define QMLARTICLEREF_H
#include "qmlref.h"
#include "article.h"

class QmlArticleRef : public QmlRef<FeedCore::Article> {
    Q_GADGET
    Q_PROPERTY(FeedCore::Article *article READ get CONSTANT);
    using QmlRef::QmlRef;
};
Q_DECLARE_METATYPE(QmlArticleRef);
#endif // QMLARTICLEREF_H
