/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QQmlNetworkAccessManagerFactory>

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    NetworkAccessManagerFactory();

    QNetworkAccessManager *create(QObject *parent) override;
};
