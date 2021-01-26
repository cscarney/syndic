#ifndef UPDATER_H
#define UPDATER_H
#include <Syndication/Feed>
#include <QObject>
#include <QDateTime>
#include <memory>

namespace FeedCore {
class Feed;

class Updater : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)
    Q_PROPERTY(FeedCore::Updater::UpdateMode updateMode READ updateMode WRITE setUpdateMode NOTIFY updateModeChanged)
    Q_PROPERTY(qint64 updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
public:
    enum UpdateMode {
        /* requests the scheduler to set the update interval to its global value */
        DefaultUpdateMode,

        /* indicates that the scheduler should use the update interval already set */
        CustomUpdateMode,

        /* indicates that we should never be scheduled */
        MaunualUpdateMode,
    };
    Q_ENUM(UpdateMode)
    Updater(Feed *feed, QObject *parent);
    ~Updater();
    virtual void run() = 0;
    virtual void abort() = 0;
    Q_INVOKABLE void start(const QDateTime &timestamp=QDateTime::currentDateTime());
    QString error();
    Feed *feed();
    QDateTime nextUpdate();
    bool hasNextUpdate();
    bool needsUpdate(const QDateTime &timestamp);
    bool updateIfNecessary(const QDateTime &timestamp);
    const QDateTime &lastUpdate();
    void setLastUpdate(const QDateTime &lastUpdate);
    UpdateMode updateMode();
    void setUpdateMode(UpdateMode updateMode);
    qint64 updateInterval();
    void setUpdateInterval(qint64 updateInterval);
    void setDefaultUpdateInterval(qint64 updateInterval);
signals:
    void lastUpdateChanged();
    void updateModeChanged();
    void updateIntervalChanged();
protected:
    void finish();
    void setError(const QString &errorMsg);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};
}

#endif // UPDATER_H
