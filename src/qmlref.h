#ifndef QMLREF_H
#define QMLREF_H
#include <QSharedPointer>
#include <QQmlEngine>

/* Encapsulates a shared pointer so that it's referent can be safely accessed from QML */
template<typename T>
class QmlRef {
public:
    QmlRef() = default;
    explicit QmlRef(const QSharedPointer<T> &ref):
        m_ref(ref)
    {
        auto *f = ref.get();
        if (f) {
            QQmlEngine::setObjectOwnership(f,QQmlEngine::CppOwnership);
        }
    }
    T *operator->() const { return m_ref.operator->(); }
    T *get() const { return m_ref.get(); }
    bool isNull() const { return m_ref.isNull(); }
    operator const QSharedPointer<T>&() const{ return m_ref(); }
protected:
    ~QmlRef() = default;
private:
    QSharedPointer<T> m_ref;
};



#endif // QMLREF_H
