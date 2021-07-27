#ifndef ENUMS_H
#define ENUMS_H
#include <QObject>

namespace FeedCore {
/**
 * Encapsulates enums so they can be accessed from QML
 */
class Enums : public QObject {
    Q_OBJECT
public:
    /**
     * update status for feeds
     */
    enum LoadStatus {
        Idle, /** < no active updates */
        Loading, /** < feed is being loaded from the storage backend */
        Updating, /** < feed is being updated from the source URL */
        Error /** < the last attempted update failed */
    };
    Q_ENUM(LoadStatus);
    Enums()=delete;
};
typedef Enums::LoadStatus LoadStatus;
}
#endif // ENUMS_H
