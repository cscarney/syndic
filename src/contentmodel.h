/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "htmlsplitter.h"
#include <QAbstractListModel>
#include <memory>

/**
 * Renders an HTML document as a list of alternating text and image blocks.
 *
 * See also HtmlSplitter
 */
class ContentModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * The HTML to be displayed
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit ContentModel(QObject *parent = nullptr);
    ~ContentModel() override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    const QString &text()
    {
        return m_text;
    }
    void setText(const QString &text);
    QHash<int, QByteArray> roleNames() const override;

signals:
    void textChanged();

protected:
    void customEvent(QEvent *event) override;

private:
    class ParseJob;
    QString m_text;
    QList<ContentBlock *> m_blocks;
    std::unique_ptr<ParseJob> m_job;
};
