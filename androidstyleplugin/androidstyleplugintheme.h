/*
    SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once
#include <Kirigami2/PlatformTheme>

class AndroidStylePluginTheme : public Kirigami::PlatformTheme
{
    Q_OBJECT

public:
    explicit AndroidStylePluginTheme(QObject *parent = nullptr);
    ~AndroidStylePluginTheme();
    QIcon iconFromTheme(const QString &name, const QColor &customColor = Qt::transparent) override;
    static void refreshInstances();

protected:
    bool event(QEvent *event) override;

private:
    void refresh();
    void updateColors();
};
