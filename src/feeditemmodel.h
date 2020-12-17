#ifndef FEEDITEMMODEL_H
#define FEEDITEMMODEL_H

#include "itemmodel.h"
#include "qmlfeedref.h"

class FeedItemModel : public ItemModel
{
    Q_OBJECT

public:
    FeedItemModel(QObject *parent=nullptr);

    FeedCore::FeedRef feed() const;
    void setFeed(const FeedCore::FeedRef &feedId);

    Q_PROPERTY(FeedCore::FeedRef feed READ feed WRITE setFeed NOTIFY feedChanged);

signals:
    void feedChanged();

    // ItemModel interface
public:
    void initialize() final;
    void requestUpdate() final;

protected:
    FeedCore::ItemQuery *startQuery() final;
    void setStatusFromUpstream() final;

private:
    FeedCore::FeedRef m_feed;

    void onStatusChanged();
};

#endif // FEEDITEMMODEL_H
