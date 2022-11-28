/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "qreadablereadability.h"
#include "networkaccessmanager.h"
#include "readabilityresult.h"
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>
#include <QString>
#include <QThread>

#include <qreadable/readable.h>

using namespace FeedCore;

class QReadableReadability::Result : public ReadabilityResult
{
    QNetworkReply *m_reply;
    QReadableReadability *m_parent;

public:
    Result(QReadableReadability *parent, QNetworkReply *reply)
        : ReadabilityResult(parent)
        , m_reply{reply}
        , m_parent{parent}
    {
        reply->setParent(this);
        QObject::connect(reply, &QNetworkReply::finished, this, &Result::onNetworkReplyFinished);
    }

    void onNetworkReplyFinished()
    {
        if (m_reply->error() != QNetworkReply::NoError) {
            emit error();
            deleteLater();
            return;
        }
        QByteArray data = m_reply->readAll();
        QString rawHtml(data);
        m_parent->parse(rawHtml, m_reply->url(), this);
    }

    // NB: Executes on worker thread
    void onGotReadabilityResult(const QString &readableHtml)
    {
        emit finished(readableHtml);
        deleteLater();
    }
};

class QReadableReadability::Worker : public QObject
{
    QReadable::Readable *m_readable{nullptr};

public:
    void parse(const QString &rawHtml, const QUrl &url, Result *result)
    {
        if (m_readable == nullptr) {
            m_readable = new QReadable::Readable(this);
        }
        QString readableHtml = m_readable->parse(rawHtml, url);
        QMetaObject::invokeMethod(result, [result, readableHtml] {
            result->onGotReadabilityResult(readableHtml);
        });
    }
};

QReadableReadability::QReadableReadability()
    : m_thread{new QThread(this)}
    , m_worker{new Worker()}
{
    m_thread->start();
    m_thread->setPriority(QThread::LowestPriority);
    m_worker->moveToThread(m_thread);
    QObject::connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
}

QReadableReadability::~QReadableReadability()
{
    m_thread->quit();
    m_thread->wait();
}

ReadabilityResult *QReadableReadability::fetch(const QUrl &url)
{
    auto *nam = NetworkAccessManager::instance();
    QNetworkRequest req(url);
    auto *reply = nam->get(req);
    return new Result(this, reply);
}

void QReadableReadability::parse(const QString &rawHtml, const QUrl &url, Result *result)
{
    QMetaObject::invokeMethod(m_worker, [worker = m_worker, rawHtml, url, result] {
        worker->parse(rawHtml, url, result);
    });
}
