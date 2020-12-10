#include "allitemmodel.h"

#include <optional>

#include "context.h"
using namespace FeedCore;

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
    return manager()->startQuery(FeedRef(), unreadFilter());
}

bool AllItemModel::itemFilter(const StoredItem &item)
{
    return true;
}

void AllItemModel::setStatusFromUpstream()
{
    if (manager()->updatesInProgress()) {
        setStatus(LoadStatus::Updating);
    } else {
        setStatus(LoadStatus::Idle);
    }
}
