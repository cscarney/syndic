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
    void initialize() override final;
    void requestUpdate() override final;

protected:
    FeedCore::ItemQuery *startQuery() override final;
    void setStatusFromUpstream() override final;


private slots:
    void slotStatusChanged();

private:
    FeedCore::FeedRef m_feed;
};

#endif // FEEDITEMMODEL_H
