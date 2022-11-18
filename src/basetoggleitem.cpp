#include "basetoggleitem.h"

BaseToggleItem::BaseToggleItem() = default;

QQmlComponent *BaseToggleItem::trueDelegate() const
{
    return m_trueDelegate.component();
}

void BaseToggleItem::setTrueDelegate(QQmlComponent *newTrueDelegate)
{
    m_trueDelegate.setComponent(newTrueDelegate, this);
}

QQmlComponent *BaseToggleItem::falseDelegate() const
{
    return m_falseDelegate.component();
}

void BaseToggleItem::setFalseDelegate(QQmlComponent *newFalseDelegate)
{
    m_falseDelegate.setComponent(newFalseDelegate, this);
}

QQuickItem *BaseToggleItem::trueItem() const
{
    return m_trueDelegate.instance();
}

QQuickItem *BaseToggleItem::falseItem() const
{
    return m_falseDelegate.instance();
}

bool BaseToggleItem::value() const
{
    return m_value;
}

void BaseToggleItem::setValue(bool newValue)
{
    if (m_value == newValue) {
        return;
    }
    m_value = newValue;
    sync();
    emit valueChanged();
}

void BaseToggleItem::sync()
{
    m_trueDelegate.setActive(m_value, this);
    m_falseDelegate.setActive(!m_value, this);
}

void BaseToggleItem::componentComplete()
{
    sync();
    QQuickItem::componentComplete();
}

void BaseToggleItem::setActiveItem(QQuickItem *item)
{
    if (m_activeItem == item) {
        return;
    }
    if (m_activeItem != nullptr) {
        emit beginAnimateHeight(item->implicitHeight());
    }
    m_activeItem = item;
    emit activeItemChanged();
}

void BaseToggleItem::Delegate::makeInstance(BaseToggleItem *parent)
{
    auto *instance = qobject_cast<QQuickItem *>(m_component->createWithInitialProperties({{"width", parent->width()}}, qmlContext(parent)));
    m_instance.reset(instance);
    m_instance->setParentItem(parent);
    QObject::connect(parent, &QQuickItem::widthChanged, instance, [parent, instance] {
        instance->setWidth(parent->width());
    });
}

void BaseToggleItem::Delegate::setComponent(QQmlComponent *newComponent, BaseToggleItem *parent)
{
    if (newComponent == m_component) {
        return;
    }
    m_component = newComponent;
    componentChanged(parent);
    if (m_isActive) {
        makeInstance(parent);
        instanceChanged(parent);
    } else if (m_instance) {
        m_instance.reset(nullptr);
        instanceChanged(parent);
    }
}

void BaseToggleItem::Delegate::setActive(bool active, BaseToggleItem *parent)
{
    if (active == m_isActive) {
        return;
    }
    m_isActive = active;
    if (active) {
        if (!m_instance) {
            makeInstance(parent);
            instanceChanged(parent);
        }
        parent->setActiveItem(m_instance.get());
    }
    m_instance->setVisible(active);
}

QQuickItem *BaseToggleItem::activeItem() const
{
    return m_activeItem;
}
