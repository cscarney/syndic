#include "rowwindowproxymodel.h"

RowWindowProxyModel::RowWindowProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

QModelIndex RowWindowProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid()) {
        return QModelIndex();
    }

    int proxyRow = proxyIndex.row();
    if (proxyRow < 0 || proxyRow >= m_windowSize) {
        return QModelIndex();
    }

    int sourceRow = proxyRow + m_windowBegin;
    return sourceModel()->index(sourceRow, proxyIndex.column(), QModelIndex());
}

QModelIndex RowWindowProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid()) {
        return QModelIndex();
    }

    int proxyRow = sourceIndex.row() - m_windowBegin;
    if (proxyRow < 0 || proxyRow >= m_windowSize) {
        return QModelIndex();
    }

    return createIndex(proxyRow, sourceIndex.column(), sourceIndex.internalPointer());
}

int RowWindowProxyModel::rowCount(const QModelIndex &parent) const
{
    if (!sourceModel() || parent.isValid()) {
        return 0;
    }

    return m_windowSize;
}

int RowWindowProxyModel::columnCount(const QModelIndex &parent) const
{
    if (!sourceModel() || parent.isValid()) {
        return 0;
    }

    return sourceModel()->columnCount();
}

QModelIndex RowWindowProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if ((sourceModel() == nullptr) || parent.isValid() || row < 0 || row >= rowCount() || column < 0 || column >= columnCount()) {
        return QModelIndex();
    }

    // Return a proxy index with the given row and column
    return createIndex(row, column);
}

QModelIndex RowWindowProxyModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

void RowWindowProxyModel::setWindowFromRow(int row)
{
    constexpr const int kRowWindowSize = 1;
    if (!sourceModel()) {
        m_windowBegin = 0;
        m_windowSize = 0;
        return;
    }
    m_windowBegin = qMax(row - kRowWindowSize, 0);
    int windowEnd = qMin(row + kRowWindowSize + 1, sourceModel()->rowCount());
    m_windowSize = windowEnd - m_windowBegin;
    setCurrentProxyRow(row - m_windowBegin);
}

int RowWindowProxyModel::currentRow() const
{
    return m_currentRow;
}

void RowWindowProxyModel::setCurrentRow(int row)
{
    if (row != m_currentRow) {
        beginResetModel();
        setWindowFromRow(row);
        endResetModel();

        m_currentRow = row;
        emit currentRowChanged();
    }
}

int RowWindowProxyModel::currentProxyRow() const
{
    return m_currentProxyRow;
}

void RowWindowProxyModel::setCurrentProxyRow(int row)
{
    if (row != m_currentProxyRow) {
        m_currentProxyRow = row;
        emit currentProxyRowChanged();
    }
}

void RowWindowProxyModel::finishModelReset()
{
    setWindowFromRow(m_currentRow);
    endResetModel();
}

void RowWindowProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (this->sourceModel() == sourceModel) {
        return;
    }

    if (this->sourceModel()) {
        QObject::disconnect(this->sourceModel(), nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(sourceModel);
    setWindowFromRow(m_currentRow);
    if (sourceModel) {
        QObject::connect(sourceModel, &QAbstractItemModel::modelAboutToBeReset, this, &RowWindowProxyModel::beginResetModel);
        QObject::connect(sourceModel, &QAbstractItemModel::modelReset, this, &RowWindowProxyModel::finishModelReset);

        QObject::connect(sourceModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &RowWindowProxyModel::beginResetModel);
        QObject::connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &RowWindowProxyModel::finishModelReset);

        QObject::connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &RowWindowProxyModel::beginResetModel);
        QObject::connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, &RowWindowProxyModel::beginResetModel);

        QObject::connect(sourceModel, &QAbstractItemModel::rowsAboutToBeMoved, this, &RowWindowProxyModel::beginResetModel);
        QObject::connect(sourceModel, &QAbstractItemModel::rowsMoved, this, &RowWindowProxyModel::finishModelReset);
    }
}
