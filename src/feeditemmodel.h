#ifndef FEEDITEMMODEL_H
#define FEEDITEMMODEL_H
#include "itemmodel.h"
#include "feedref.h"
class FeedItemModel : public ItemModel
{
    Q_OBJECT
public:
    FeedItemModel(QObject *parent=nullptr);
    FeedCore::FeedRef feed() const;
    void setFeed(const FeedCore::FeedRef &feedId);
    void initialize() final;
    void requestUpdate() final;
    Q_PROPERTY(FeedCore::FeedRef feed READ feed WRITE setFeed NOTIFY feedChanged);
signals:
    void feedChanged();
protected:
    FeedCore::Future<FeedCore::ArticleRef> *startQuery() final;
    void setStatusFromUpstream() final;
private:
    FeedCore::FeedRef m_feed;
    void onStatusChanged();
};
#endif // FEEDITEMMODEL_H
