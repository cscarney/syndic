#ifndef ALLITEMMODEL_H
#define ALLITEMMODEL_H

#include "itemmodel.h"

class AllItemModel : public ItemModel
{
    Q_OBJECT
public:
    AllItemModel(QObject *parent=nullptr);

    // ItemModel interface
public:
    void initialize() override;
    void requestUpdate() override final;

protected:
    FeedCore::ItemQuery *startQuery() override final;
    void setStatusFromUpstream() override final;
};

#endif // ALLITEMMODEL_H
