#ifndef FEEDITEMMODEL_H
#define FEEDITEMMODEL_H

#include "itemmodel.h"

class FeedItemModel : public ItemModel
{
    Q_OBJECT

public:
    FeedItemModel(QObject *parent=nullptr);

    FeedCore::FeedRef feed() const;
    void setFeed(const FeedCore::FeedRef &feedId);

    void setFeedWrapper(const FeedCore::FeedRefWrapper &feed);
    FeedCore::FeedRefWrapper feedWrapper() const;
    Q_PROPERTY(FeedCore::FeedRefWrapper feed READ feedWrapper WRITE setFeedWrapper NOTIFY feedChanged);

signals:
    void feedChanged();

    // ItemModel interface
public:
    void requestUpdate() override final;

protected:
    FeedCore::ItemQuery *startQuery() override final;
    bool itemFilter(const FeedCore::StoredItem &item) override final;
    void setStatusFromUpstream() override final;

private:
    FeedCore::FeedRef m_feed;
};

#endif // FEEDITEMMODEL_H
