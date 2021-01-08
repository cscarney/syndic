#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H
#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <memory>
#include "future.h"
#include "feedref.h"
#include "articleref.h"
namespace FeedCore {
class Context;
}

class FeedListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(FeedCore::Context *context READ context WRITE setContext NOTIFY contextChanged);
public:
    explicit FeedListModel(QObject *parent = nullptr);
    ~FeedListModel();
    enum Roles {
        Ref = Qt::UserRole,
        Icon,
    };
    Q_ENUM(Roles);
    FeedCore::Context *context() const;
    void setContext(FeedCore::Context *context);
    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const final;
    void classBegin() override;
    void componentComplete() override;
signals:
    void contextChanged();
private:
    class PrivData;
    std::unique_ptr<PrivData> priv;
    void onGetFeedsFinished(FeedCore::Future<FeedCore::FeedRef> *sender);
    void onFeedAdded(const FeedCore::FeedRef &feed);
};
#endif // FEEDLISTMODEL_H
