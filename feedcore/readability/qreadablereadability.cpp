/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qreadablereadability.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

using namespace FeedCore;

static const QString executableName = QStringLiteral("qreadable");

QReadableReadability::QReadableReadability()
{
    // we shouldn't be creating this class unless we already checked for qreadable
    bool available = isSupported();
    if (Q_UNLIKELY(!available)) {
        qWarning() << "Creating instance of QReadableReadability when qreadable is not available";
    }
    setAvailable(available);
}

bool QReadableReadability::isSupported()
{
    static const bool supported = !QStandardPaths::findExecutable(executableName).isEmpty();
    return supported;
}

QString QReadableReadability::program() const
{
    return executableName;
}

QStringList QReadableReadability::arguments(const QString &url) const
{
    return {url};
}
