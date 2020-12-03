#include <QTimer>

#include "managedlistmodel.h"

ManagedListModel::ManagedListModel(QObject *parent) :
    QAbstractListModel(parent)
{

}

FeedManager *ManagedListModel::manager() const
{
    return m_manager;
}

void ManagedListModel::setManager(FeedManager *manager)
{
    assert(!active());
    m_manager = manager;
    managerChanged();
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
    QTimer::singleShot(0,this, &ManagedListModel::activate);
}
