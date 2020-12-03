#ifndef MANAGEDLISTMODEL_H
#define MANAGEDLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlParserStatus>

class FeedManager;

class ManagedListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
public:
    ManagedListModel(QObject *parent=nullptr);

    FeedManager *manager() const;
    void setManager(FeedManager *manager);
    Q_PROPERTY(FeedManager *manager READ manager WRITE setManager NOTIFY managerChanged);

    inline bool active() { return m_active; }
    void activate();

    void classBegin() override;
    void componentComplete() override;

signals:
    void managerChanged();

protected:
    virtual void initialize() {};

private:
    FeedManager *m_manager;
    bool m_active;
};

#endif // MANAGEDLISTMODEL_H
