/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QFuture>
#include <QObject>

namespace FeedCore::Future
{
/**
 **
 * Create a QPromise/QFuture pair with a single-shot timer
 */
template<typename T, typename Callable>
QFuture<T> yield(QObject *context, Callable call)
{
    QPromise<T> promise;
    QFuture result = promise.future();
    QMetaObject::invokeMethod(
        context,
        [context = QPointer<QObject>(context), promise = std::move(promise), call]() mutable {
            promise.start();
            call(promise);
            promise.finish();
        },
        Qt::QueuedConnection);
    return result;
}

/**
 * A wrapper around QFuture::then that guards against the context
 * object being destroyed before the future completes.
 */
template<typename T, typename Functor>
void safeThen(QFuture<T> &f, QObject *target, Functor func)
{
    f.then([target = QPointer<QObject>(target), func](QFuture<T> f) {
        if (target) {
            QMetaObject::invokeMethod(target.get(), [f, func] {
                func(f);
            });
        }
    });
}

template<typename T>
QList<T> safeResults(const QFuture<T> &f)
{
    return f.isValid() ? f.results() : QList<T>();
}
}
