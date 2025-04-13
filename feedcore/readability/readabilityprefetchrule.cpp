/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "readabilityprefetchrule.h"
#include "article.h"
#include "feed.h"
#include "readability.h"
#include "readabilityresult.h"
#include <QFlags>
#include <QPromise>

using namespace FeedCore;

ReadabilityPrefetchRule::ReadabilityPrefetchRule(Readability *readability, QObject *parent)
    : AbstractAutomationRule(parent)
    , m_readability(readability)
{
}

bool ReadabilityPrefetchRule::matches(const ArticleRef &article)
{
    if (!article->feed()) {
        // feed has been deleted, don't match
        return false;
    };
    // check if we have already fetched the content
    return QFlags<Feed::FeedFlags>(article->feed()->flags()).testFlag(Feed::UseReadableContentFlag);
}

QFuture<void> ReadabilityPrefetchRule::beginPerformAction(const ArticleRef &article)
{
    // make sure we still match when it's time to run
    if (!matches(article)) {
        return QtFuture::makeReadyVoidFuture();
    }

    auto *promise = new QPromise<void>();
    promise->start();

    ReadabilityResult *result = m_readability->fetch(article->url());

    QObject::connect(result, &ReadabilityResult::finished, result, [article, promise](const QString &content) {
        article->cacheReadableContent(content);
        promise->finish();
        delete promise;
    });

    QObject::connect(result, &ReadabilityResult::error, result, [promise, article]() {
        qDebug() << "Readability content fetch failed for " << article->url();
        promise->finish();
        delete promise;
    });

    return promise->future();
}
