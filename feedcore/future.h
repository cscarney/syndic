/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include <QObject>
#include <QTimer>
#include <QVector>

namespace FeedCore
{
class BaseFuture : public QObject
{
    Q_OBJECT
signals:
    void finished();
};

/**
 * A quick-and-dirty single-thread async class.
 *
 * This should probably be replaced with QPromise/QFuture at some point.
 */
template<typename T>
class Future : public BaseFuture
{
public:
    const QVector<T> &result()
    {
        return m_result;
    };
    void setResult(const QVector<T> &&result)
    {
        m_result = result;
    };
    void setResult(const T &result)
    {
        m_result = {result};
    };
    void setResult()
    {
        m_result = {};
    };
    void appendResult(const T &result)
    {
        m_result << result;
    };

    template<typename Callable>
    static Future<T> *yield(QObject *context, Callable call)
    {
        auto *op = new Future<T>;
        QTimer::singleShot(0, context, [op, call] {
            call(op);
            op->finish();
        });
        return op;
    }

protected:
    void finish()
    {
        emit finished();
        delete this;
    }

private:
    QVector<T> m_result;
};

template<typename T>
class UnionFuture : public Future<T>
{
public:
    void addFuture(Future<T> *f)
    {
        m_pending++;
        QObject::connect(f, &BaseFuture::finished, this, [this, f] {
            m_pending--;
            for (auto i : f->result()) {
                this->appendResult(i);
            }
            if (!m_pending) {
                this->finish();
            }
        });
    }

    template<typename Callable>
    static UnionFuture<T> *create(Callable call)
    {
        auto *op = new UnionFuture<T>;
        call(op);
        op->scheduleIfEmpty();
        return op;
    }

private:
    int m_pending{0};
    UnionFuture() = default;

    void scheduleIfEmpty()
    {
        if (!m_pending) {
            QTimer::singleShot(0, this, &UnionFuture<T>::finish);
        }
    }
};
}
