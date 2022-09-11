/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FEEDCORE_QPROCESSREADABILITY_H
#define FEEDCORE_QPROCESSREADABILITY_H

#include "readability.h"

#include <QQueue>
#include <QString>
#include <QStringList>

class QProcess;

namespace FeedCore
{

class QProcessReadability : public Readability
{
    Q_OBJECT
public:
    QProcessReadability();
    virtual ~QProcessReadability() = default;

    void fetch(const QString &url) override;

protected:
    void setAvailable(bool available);
    virtual QString program() const = 0;
    virtual QStringList arguments(const QString &url) const = 0;

private:
    void start(QProcess &process, const QString &url);

    struct ReadabilityProcess {
        QString m_url;
        QProcess *m_process;
    };

    bool m_isAvailable;
    QQueue<ReadabilityProcess> m_processes;
};
}

#endif
