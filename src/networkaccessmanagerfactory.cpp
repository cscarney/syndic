/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "networkaccessmanagerfactory.h"
#include "networkaccessmanager.h"

NetworkAccessManagerFactory::NetworkAccessManagerFactory() = default;

QNetworkAccessManager *NetworkAccessManagerFactory::create(QObject *parent)
{
    return new FeedCore::NetworkAccessManager(parent);
}
