#ifndef ALLITEMMODEL_H
#define ALLITEMMODEL_H

#include "loadstatus.h"
#include "itemmodel.h"

class AllItemModel : public ItemModel
{
    Q_OBJECT
public:
    AllItemModel(QObject *parent=nullptr);

    // ItemModel interface
public:
    void requestUpdate() override final;

protected:
    ItemQuery *startQuery() override final;
    bool itemFilter(const StoredItem &item) override final;
    void setStatusFromUpstream() override final;
};

#endif // ALLITEMMODEL_H
