/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "contentmodel.h"

#include <QCoreApplication>
#include <QEvent>
#include <memory>

namespace
{
const auto kAddBlockEventType = static_cast<QEvent::Type>(QEvent::registerEventType());

class AddBlockEvent : public QEvent
{
    std::unique_ptr<ContentBlock> m_contentBlock;

public:
    explicit AddBlockEvent(ContentBlock *block)
        : QEvent(kAddBlockEventType)
        , m_contentBlock{block}
    {
    }

    ContentBlock *takeBlock()
    {
        return m_contentBlock.release();
    }
};
}

ContentModel::ContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

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
    if (Q_LIKELY(role == Qt::UserRole)) {
        return QVariant::fromValue(block);
    }
    return QVariant();
}

void ContentModel::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        beginResetModel();
        QCoreApplication::removePostedEvents(this, kAddBlockEventType);
        qDeleteAll(m_blocks);
        m_blocks.clear();
        endResetModel();
        emit textChanged();

        const QVector<ContentBlock *> blocks = HtmlSplitter::cleanHtml(text, nullptr);
        for (ContentBlock *block : blocks) {
            QCoreApplication::postEvent(this, new AddBlockEvent(block), Qt::LowEventPriority);
        }
    }
}

QHash<int, QByteArray> ContentModel::roleNames() const
{
    return {{Qt::UserRole, "block"}};
}

void ContentModel::customEvent(QEvent *event)
{
    if (auto *addBlockEvent = dynamic_cast<AddBlockEvent *>(event)) {
        int idx = m_blocks.count();
        beginInsertRows(QModelIndex(), idx, idx);
        ContentBlock *block = addBlockEvent->takeBlock();
        block->setParent(this);
        m_blocks.append(block);
        endInsertRows();
    } else {
        QAbstractListModel::customEvent(event);
    }
}
