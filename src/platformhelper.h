/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PLATFORMHELPER_H
#define PLATFORMHELPER_H

#include <QObject>
#include <QUrl>

class PlatformHelper : public QObject
{
    Q_OBJECT
public:
    explicit PlatformHelper(QObject *parent = nullptr);
    Q_INVOKABLE static void share(const QUrl &url);
};

#endif // PLATFORMHELPER_H
