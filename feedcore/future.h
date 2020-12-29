#ifndef FEEDSTORAGEOPERATION_H
#define FEEDSTORAGEOPERATION_H
#include <QObject>
#include <QVector>
#include <QTimer>

namespace FeedCore {
class ArticleRef;

class BaseFuture: public QObject {
    Q_OBJECT
signals:
    void finished();
};

template<typename T>
class Future : public BaseFuture {
public:
    inline const QVector<T> &result(){ return m_result; };
    inline void setResult(const QVector<T> &&result) { m_result = result; };
    inline void setResult(const T &result ) { m_result = {result}; };
    inline void setResult() { m_result = {}; };
    inline void appendResult(const T &result) { m_result << result; };

    template<typename Callable>
    static inline Future<T> *yield(QObject *context, Callable call)
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
    QVector<T> m_result = {};
};
}
#endif // FEEDSTORAGEOPERATION_H
