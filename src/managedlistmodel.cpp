#include "managedlistmodel.h"
#include <QTimer>

using namespace FeedCore;

ManagedListModel::ManagedListModel(QObject *parent) :
    QAbstractListModel(parent)
{

}

Context *ManagedListModel::manager() const
{
    return m_manager;
}

void ManagedListModel::setManager(Context *manager)
{
    assert(!active());
    m_manager = manager;
    emit managerChanged();
}

void ManagedListModel::activate()
{
    assert(manager() != nullptr);
    m_active = true;
    initialize();
}

void ManagedListModel::classBegin()
{

}

void ManagedListModel::componentComplete()
{
    // make sure we don't activate until the settings item is complete
    QTimer::singleShot(0,this,&ManagedListModel::activate);
}
