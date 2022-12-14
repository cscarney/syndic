/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QSharedPointer>

namespace FeedCore
{
class Article;
typedef QSharedPointer<Article> ArticleRef;
}
