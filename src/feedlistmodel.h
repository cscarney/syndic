#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H
#include <memory>
#include "managedlistmodel.h"
#include "future.h"
namespace FeedCore {
class ArticleRef;
class FeedRef;
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
    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const final;
private:
    class PrivData;
    std::unique_ptr<PrivData> priv;
    void onFeedQueryFinished(FeedCore::Future<FeedCore::FeedRef> *sender);
    void onFeedAdded(const FeedCore::FeedRef &feed);
};
#endif // FEEDLISTMODEL_H
