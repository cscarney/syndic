#include <optional>

#include "allitemmodel.h"
#include "feedmanager.h"

AllItemModel::AllItemModel(QObject *parent):
    ItemModel(parent)
{

}


ItemQuery *AllItemModel::startQuery()
{
    return manager()->startQuery(std::nullopt, unreadFilter());
}

bool AllItemModel::itemFilter(const StoredItem &item)
{
    return true;
}

void AllItemModel::setStatusFromUpstream()
{
    if (manager()->updatesInProgress()) {
        setStatus(Updating);
    } else {
        setStatus(Ok);
    }
}
