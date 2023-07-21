/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "qqmlintegration.h"
#include <QJSValue>
#include <QObject>
#include <QPointer>

class PageSpec
{
    Q_GADGET
    Q_PROPERTY(QString componentName READ componentName WRITE setComponentName)
    Q_PROPERTY(QJSValue pageData READ pageData WRITE setPageData)

public:
    QString componentName() const;
    void setComponentName(const QString &newComponentName);

    QJSValue pageData() const;
    void setPageData(const QJSValue &newPageData);

    bool operator==(const PageSpec &other);

private:
    QString m_componentName;
    QJSValue m_pageData;
};
Q_DECLARE_METATYPE(PageSpec);

class PageControlAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *pageController READ pageController NOTIFY pageControllerChanged)
    Q_PROPERTY(PageSpec nextPage READ nextPage WRITE setNextPage NOTIFY nextPageChanged)
    QML_ATTACHED(PageControlAttached)

public:
    QObject *pageController() const;
    Q_INVOKABLE void setPageController(QObject *newPageController);
    PageSpec nextPage() const;
    void setNextPage(const PageSpec &newPageSpec);
    Q_INVOKABLE void sync();

    static PageControlAttached *qmlAttachedProperties(QObject *object)
    {
        return new PageControlAttached(object);
    }

signals:
    void pageControllerChanged();
    void nextPageChanged();

private:
    explicit PageControlAttached(QObject *parent);

    PageSpec m_nextPage;
    QPointer<QObject> m_pageController;
};
