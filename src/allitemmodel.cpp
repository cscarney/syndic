#include "allitemmodel.h"

#include <optional>

#include "context.h"
using namespace FeedCore;

AllItemModel::AllItemModel(QObject *parent):
    ItemModel(parent)
{

}

void AllItemModel::initialize() {
    auto *m = manager();
    QObject::connect(m, &Context::itemAdded, this, &AllItemModel::slotItemAdded);
    QObject::connect(m, &Context::itemChanged, this, &AllItemModel::slotItemChanged);
    refresh();
}


void AllItemModel::requestUpdate()
{
    manager()->requestUpdate();
}


ItemQuery *AllItemModel::startQuery()
{
    return manager()->startQuery(FeedRef(), unreadFilter());
}

void AllItemModel::setStatusFromUpstream()
{
    if (manager()->updatesInProgress()) {
        setStatus(LoadStatus::Updating);
    } else {
        setStatus(LoadStatus::Idle);
    }
}
