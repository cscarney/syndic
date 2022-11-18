/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "objectcollectionmodel.h"
#include <QNetworkReply>
#include <QSharedPointer>
#include <memory>

class Foo : ObjectCollectionModel<QSharedPointer<QNetworkReply>>
{
};
