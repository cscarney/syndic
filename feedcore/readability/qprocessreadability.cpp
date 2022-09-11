/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qreadablereadability.h"

#include <QDebug>
#include <QProcess>

using namespace FeedCore;

QProcessReadability::QProcessReadability()
    : m_isAvailable(false)
{
}

void QProcessReadability::fetch(const QString &url)
{
    if (m_isAvailable) {
        ReadabilityProcess readabilityProcess{url, new QProcess()};
        m_processes.enqueue(readabilityProcess);
        connect(readabilityProcess.m_process,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                [this, readabilityProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                    Q_UNUSED(exitCode)
                    if (exitStatus == QProcess::NormalExit) {
                        const QString readable = QString::fromUtf8(readabilityProcess.m_process->readAll());
                        if (!readable.isEmpty()) {
                            Q_EMIT finishedFetching(readabilityProcess.m_url, readable);
                        } else {
                            Q_EMIT errorFetching(readabilityProcess.m_url);
                        }
                    } else {
                        Q_EMIT errorFetching(readabilityProcess.m_url);
                    }
                    delete readabilityProcess.m_process;
                    m_processes.dequeue();

                    // start next
                    if (!m_processes.empty()) {
                        const ReadabilityProcess &nextProcess = m_processes.head();
                        start(*nextProcess.m_process, nextProcess.m_url);
                    }
                });

        // if no process is running, start it
        if (m_processes.size() == 1) {
            start(*readabilityProcess.m_process, url);
        }
    }
}

void QProcessReadability::start(QProcess &process, const QString &url)
{
    process.start(program(), arguments(url));

    Q_EMIT startedFetching(url);
}

void QProcessReadability::setAvailable(bool available)
{
    m_isAvailable = available;
}
