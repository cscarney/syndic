/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_FUTURE_H
#define FEEDCORE_FUTURE_H
#include <QObject>
#include <QVector>
#include <QTimer>
#include "articleref.h"

namespace FeedCore {
class BaseFuture: public QObject {
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
class Future : public BaseFuture {
public:
    const QVector<T> &result(){ return m_result; };
    void setResult(const QVector<T> &&result) { m_result = result; };
    void setResult(const T &result ) { m_result = {result}; };
    void setResult() { m_result = {}; };
    void appendResult(const T &result) { m_result << result; };

    template<typename Callable>
    static Future<T> *yield(QObject *context, Callable call)
    {
        auto *op = new Future<T>;
        QTimer::singleShot(0, context, [op, call]{
            call(op);
            emit op->finished();
            delete op;
        });
        return op;
    }
private:
    QVector<T> m_result;
};
}
#endif // FEEDCORE_FUTURE_H
