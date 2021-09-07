/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_ARTICLEREF_H
#define FEEDCORE_ARTICLEREF_H
#include <QSharedPointer>

namespace FeedCore
{
class Article;
typedef QSharedPointer<Article> ArticleRef;
}
#endif // FEEDCORE_ARTICLEREF_H
