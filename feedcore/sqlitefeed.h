#ifndef SQLITEFEED_H
#define SQLITEFEED_H

#include "feed.h"

class QSqlQuery;

namespace FeedCore {

class SqliteFeed : public Feed {
    Q_OBJECT
public:
    static QSharedPointer<SqliteFeed> forId(qint64 feedId);
    static QSharedPointer<SqliteFeed> fromQuery(const QSqlQuery &q);

    qint64 id();

    void updateFromQuery(const QSqlQuery &query);
    void populateNew(const QUrl &url, const QString &name);

private:

    SqliteFeed(qint64 feedId);

    qint64 m_id;
};

}

#endif // SQLITEFEED_H
