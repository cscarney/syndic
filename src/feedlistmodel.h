#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include <memory>

#include "managedlistmodel.h"
#include "feedstorageoperation.h"
#include "enums.h"
#include "feedref.h"

namespace FeedCore {
class ArticleRef;
}

class FeedListModel : public ManagedListModel
{
    Q_OBJECT
public:
    explicit FeedListModel(QObject *parent = nullptr);
    ~FeedListModel();

    enum Roles {
        Ref = Qt::UserRole,
        Icon,
    };
    Q_ENUM(Roles);

    void initialize() override;

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    class PrivData;
    std::unique_ptr<PrivData> priv;

    void onFeedQueryFinished(FeedCore::FeedQuery *sender);
    void onFeedAdded(const FeedCore::FeedRef &feed);
};

#endif // FEEDLISTMODEL_H
