/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CONTENTMODEL_H
#define CONTENTMODEL_H
#include <QAbstractListModel>
#include "htmlsplitter.h"

class ContentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    ContentModel(QObject *parent=nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    const QString &text() { return m_text; }
    void setText(const QString& text);
    QHash<int, QByteArray> roleNames() const override;

signals:
    void textChanged();
private:
    QString m_text;
    QVector<ContentBlock*> m_blocks;
};

#endif // CONTENTMODEL_H
