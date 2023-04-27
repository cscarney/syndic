/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "contentmodel.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMutex>
#include <QPointer>
#include <QRunnable>
#include <QThreadPool>
#include <memory>
#include <utility>

#include <QDebug>

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

    ContentBlock *takeBlock(QObject *newParent)
    {
        m_contentBlock->setParent(newParent);
        return m_contentBlock.release();
    }
};
}

class ContentModel::ParseJob : public QRunnable
{
    QString m_text;
    QMutex m_runLock;
    std::atomic_bool m_cancelled{false};
    QObject *m_target;

public:
    Q_DISABLE_COPY_MOVE(ParseJob)
    ParseJob(QObject *target, QString text)
        : m_text(std::move(text))
        , m_target(target)
    {
        setAutoDelete(false);
    }

    ~ParseJob() override
    {
        // block destruction while job is running
        m_cancelled = true;
        QMutexLocker lock(&m_runLock);
    }

    void run() override
    {
        QMutexLocker lock(&m_runLock);
        const QList<ContentBlock *> blocks = HtmlSplitter::cleanHtml(m_text, nullptr);
        for (ContentBlock *block : blocks) {
            if (m_cancelled) {
                return;
            }
            block->moveToThread(m_target->thread());
            QCoreApplication::postEvent(m_target, new AddBlockEvent(block), Qt::LowEventPriority);
        }
    }
};

ContentModel::ContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ContentModel::~ContentModel() = default;

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

        for (auto *block : qAsConst(m_blocks)) {
            block->deleteLater();
        }
        beginResetModel();
        m_blocks.clear();
        endResetModel();

        if (!m_text.isEmpty()) {
            m_job = std::make_unique<ParseJob>(this, text);
            QCoreApplication::removePostedEvents(this, kAddBlockEventType);
            QThreadPool::globalInstance()->start(m_job.get());
        }

        emit textChanged();
    }
}

QHash<int, QByteArray> ContentModel::roleNames() const
{
    return {{Qt::UserRole, "block"}};
}

void ContentModel::customEvent(QEvent *event)
{
    if (auto *addBlockEvent = dynamic_cast<AddBlockEvent *>(event)) {
        assert(QThread::currentThread() == this->thread());
        int idx = m_blocks.count();
        beginInsertRows(QModelIndex(), idx, idx);
        ContentBlock *block = addBlockEvent->takeBlock(this);
        m_blocks.append(block);
        endInsertRows();
    } else {
        QAbstractListModel::customEvent(event);
    }
}
