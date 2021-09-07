/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef QMLREF_H
#define QMLREF_H
#include <QQmlEngine>
#include <QSharedPointer>

/**
 *  Encapsulates a shared pointer so that it's referent can be safely accessed from QML
 *
 *  This works around several problems when interfacing between QSharedPointer and QML:
 *  (a) it prevents the QML engine from taking ownership of the shared object and deleting it
 *          from under the shared pointer.
 *  (b) it prevents shared objects from being deleted out from under the QML engine when
 *          the last C++ reference is gone
 *  (c) it allows shared objects to pass through QML to interfaces that expect a QSharedPointer
 *
 *  Care must be taken to avoid reference cycles; QML's garbage collector will not break
 *  cycles that run through a QmlRef object.
 *
 *  This template class cannot be registered with the QML engine directly.  Create a template
 *  specialization with a Q_GADGET macro for each type you need, e.g.
 *
 *      class QmlFooRef : public QmlRef<Foo> {
 *          Q_GADGET
 *          Q_PROPERTY(Foo *get READ get CONSTANT);
 *          using QmlRef::QmlRef;
 *      }
 *      Q_DECLARE_METATYPE(QmlFooRef);
 */
template<typename T>
class QmlRef
{
public:
    QmlRef() = default;
    explicit QmlRef(const QSharedPointer<T> &ref)
        : m_ref(ref)
    {
        auto *f = ref.get();
        if (f) {
            QQmlEngine::setObjectOwnership(f, QQmlEngine::CppOwnership);
        }
    }
    ~QmlRef() = default;
    T *operator->() const
    {
        return m_ref.operator->();
    }
    T *get() const
    {
        return m_ref.get();
    }
    bool isNull() const
    {
        return m_ref.isNull();
    }
    operator const QSharedPointer<T> &() const
    {
        return m_ref;
    }
    operator T *() const
    {
        return m_ref.get();
    }

private:
    QSharedPointer<T> m_ref;
};

#endif // QMLREF_H
