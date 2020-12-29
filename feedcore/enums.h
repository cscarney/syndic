#ifndef LOADSTATUS_H
#define LOADSTATUS_H
#include <QObject>

namespace FeedCore {
class Enums : public QObject {
    Q_OBJECT
public:
    enum LoadStatus {
        Idle,
        Loading,
        Updating,
        Error
    };
    Q_ENUM(LoadStatus);
    Enums()=delete;
};
typedef Enums::LoadStatus LoadStatus;
}
#endif // LOADSTATUS_H
