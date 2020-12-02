#include <optional>

#include "allitemmodel.h"
#include "feedmanager.h"

AllItemModel::AllItemModel(QObject *parent):
    ItemModel(parent)
{

}

void AllItemModel::requestUpdate()
{
    manager()->requestUpdate();
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
