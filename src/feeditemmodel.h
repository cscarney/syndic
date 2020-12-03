#ifndef FEEDITEMMODEL_H
#define FEEDITEMMODEL_H

#include "itemmodel.h"

class FeedItemModel : public ItemModel
{
    Q_OBJECT

public:
    FeedItemModel(QObject *parent=nullptr);

    qint64 feedId();
    void setFeedId(qint64 feedId);
    Q_PROPERTY(qint64 feedId READ feedId WRITE setFeedId NOTIFY feedIdChanged);


signals:
    void feedIdChanged();

    // ItemModel interface
public:
    void requestUpdate() override final;

protected:
    ItemQuery *startQuery() override final;
    bool itemFilter(const StoredItem &item) override final;
    void setStatusFromUpstream() override final;

private:
    qint64 m_feedId;
};

#endif // FEEDITEMMODEL_H
