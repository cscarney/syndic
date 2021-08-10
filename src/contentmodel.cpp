#include "contentmodel.h"

ContentModel::ContentModel(QObject *parent) :
    QAbstractListModel(parent)
{}


int ContentModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_blocks.length();
}

QVariant ContentModel::data(const QModelIndex &index, int role) const
{
    ContentBlock *block = m_blocks[index.row()];
    return QVariant::fromValue(block);
}

void ContentModel::setText(const QString& text)
{
    if (m_text != text) {
        m_text = text;
        beginResetModel();
        m_blocks = HtmlSplitter::cleanHtml(text, this);
        endResetModel();
        emit textChanged();
    }
}

QHash<int, QByteArray> ContentModel::roleNames() const
{
    return {{Qt::UserRole, "block"}};
}
