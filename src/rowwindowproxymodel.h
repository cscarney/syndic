#pragma once

#include <QAbstractProxyModel>
#include <QModelIndex>

class RowWindowProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

    /**
     * The row of the source model that we are building the window around
     */
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

    /*
     * The row in the proxy model that currentRow is mapped to
     */
    Q_PROPERTY(int currentProxyRow READ currentProxyRow NOTIFY currentProxyRowChanged)
public:
    // Constructor that takes a parent object and an initial row index
    explicit RowWindowProxyModel(QObject *parent = nullptr);

    int currentRow() const;
    void setCurrentRow(int row);
    int currentProxyRow() const;

    // Reimplemented from QAbstractProxyModel
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

signals:
    void currentRowChanged();
    void currentProxyRowChanged();

private:
    int m_currentRow{0};
    int m_windowBegin{0};
    int m_windowSize{0};
    int m_currentProxyRow{0};

    void setWindowFromRow(int row);
    void setCurrentProxyRow(int row);
    void finishModelReset();
};
