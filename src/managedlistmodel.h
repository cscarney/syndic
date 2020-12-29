#ifndef MANAGEDLISTMODEL_H
#define MANAGEDLISTMODEL_H
#include <QAbstractListModel>
#include <QQmlParserStatus>

namespace FeedCore {
class Context;
}

class ManagedListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(FeedCore::Context *manager READ manager WRITE setManager NOTIFY managerChanged);
public:
    ManagedListModel(QObject *parent=nullptr);
    FeedCore::Context *manager() const;
    void setManager(FeedCore::Context *manager);
    inline bool active() { return m_active; }
    void activate();
    void classBegin() override;
    void componentComplete() override;
signals:
    void managerChanged();
protected:
    virtual void initialize() {};
private:
    FeedCore::Context *m_manager { nullptr };
    bool m_active { false };
};

#endif // MANAGEDLISTMODEL_H
