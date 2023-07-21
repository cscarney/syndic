#include "pagecontrolattached.h"
#include <QVariant>

PageControlAttached::PageControlAttached(QObject *parent)
    : QObject(parent)
{
}

PageSpec PageControlAttached::nextPage() const
{
    return m_nextPage;
}

void PageControlAttached::setNextPage(const PageSpec &newPageSpec)
{
    if (m_nextPage == newPageSpec)
        return;
    m_nextPage = newPageSpec;
    emit nextPageChanged();
    sync();
}

void PageControlAttached::sync()
{
    if (!m_pageController) {
        return;
    }
    QObject *attachedTo = this->parent();
    QMetaObject::invokeMethod(m_pageController, "sync", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(attachedTo)));
}

QObject *PageControlAttached::pageController() const
{
    return m_pageController;
}

void PageControlAttached::setPageController(QObject *newPageController)
{
    if (m_pageController != newPageController) {
        m_pageController = newPageController;
        emit pageControllerChanged();
    }
}

QString PageSpec::componentName() const
{
    return m_componentName;
}

void PageSpec::setComponentName(const QString &newComponentName)
{
    if (m_componentName == newComponentName) {
        return;
    }
    m_componentName = newComponentName;
}

QJSValue PageSpec::pageData() const
{
    return m_pageData;
}

void PageSpec::setPageData(const QJSValue &newPageData)
{
    if (m_pageData.strictlyEquals(newPageData)) {
        return;
    }
    m_pageData = newPageData;
}

bool PageSpec::operator==(const PageSpec &other)
{
    return m_componentName == other.componentName() && m_pageData.strictlyEquals(other.pageData());
}
