/*
    SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ANDROIDSTYLEPLUGINTHEME_H
#define ANDROIDSTYLEPLUGINTHEME_H

#include <Kirigami2/PlatformTheme>

class AndroidStylePluginTheme : public Kirigami::PlatformTheme
{
    Q_OBJECT

public:
    explicit AndroidStylePluginTheme(QObject *parent = nullptr);

protected:
    bool event(QEvent *event) override;

private:
    void updateColors();
};

#endif
