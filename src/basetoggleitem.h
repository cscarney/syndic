#ifndef BASETOGGLEITEM_H
#define BASETOGGLEITEM_H

#include <QQuickItem>

class BaseToggleItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent *trueDelegate READ trueDelegate WRITE setTrueDelegate NOTIFY trueDelegateChanged)
    Q_PROPERTY(QQmlComponent *falseDelegate READ falseDelegate WRITE setFalseDelegate NOTIFY falseDelegateChanged)
    Q_PROPERTY(QQuickItem *trueItem READ trueItem NOTIFY trueItemChanged)
    Q_PROPERTY(QQuickItem *falseItem READ falseItem NOTIFY falseItemChanged)
    Q_PROPERTY(QQuickItem *activeItem READ activeItem NOTIFY activeItemChanged)
    Q_PROPERTY(bool value READ value WRITE setValue NOTIFY valueChanged)

public:
    BaseToggleItem();

    QQmlComponent *trueDelegate() const;
    void setTrueDelegate(QQmlComponent *newTrueDelegate);
    QQmlComponent *falseDelegate() const;
    void setFalseDelegate(QQmlComponent *newFalseDelegate);
    QQuickItem *trueItem() const;
    QQuickItem *falseItem() const;
    bool value() const;
    void setValue(bool newValue);
    QQuickItem *activeItem() const;

signals:
    void beginAnimateHeight(qreal targetHeight);
    void trueDelegateChanged();
    void falseDelegateChanged();
    void trueItemChanged();
    void falseItemChanged();
    void valueChanged();
    void activeItemChanged();

private:
    class Delegate
    {
        QQmlComponent *m_component{nullptr};
        std::unique_ptr<QQuickItem> m_instance{nullptr};
        bool m_isActive{false};
        void makeInstance(BaseToggleItem *parent);

    public:
        virtual ~Delegate() = default;
        QQmlComponent *component() const
        {
            return m_component;
        }
        void setComponent(QQmlComponent *newComponent, BaseToggleItem *parent);
        QQuickItem *instance() const
        {
            return m_instance.get();
        };
        bool isActive() const
        {
            return m_isActive;
        }
        void setActive(bool active, BaseToggleItem *parent);
        virtual void instanceChanged(BaseToggleItem *parent) = 0;
        virtual void componentChanged(BaseToggleItem *parent) = 0;
    };

    class TrueDelegate : public Delegate
    {
        void instanceChanged(BaseToggleItem *parent) final
        {
            emit parent->trueItemChanged();
        }
        void componentChanged(BaseToggleItem *parent) final
        {
            emit parent->trueDelegateChanged();
        }
    };

    class FalseDelegate : public Delegate
    {
        void instanceChanged(BaseToggleItem *parent) final
        {
            emit parent->falseItemChanged();
        }
        void componentChanged(BaseToggleItem *parent) final
        {
            emit parent->falseDelegateChanged();
        }
    };

    TrueDelegate m_trueDelegate;
    FalseDelegate m_falseDelegate;
    bool m_value{false};
    QPointer<QQuickItem> m_activeItem{nullptr};

    void sync();
    void componentComplete() override;
    void setActiveItem(QQuickItem *item);
};

#endif // BASETOGGLEITEM_H
