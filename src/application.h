#ifndef APPLICATION_H
#define APPLICATION_H
#include <QApplication>
#include <memory>
#include <QQmlApplicationEngine>

namespace FeedCore {
class Context;
}
class Settings;
class NotificationController;

typedef QApplication ApplicationBaseClass;

class Application : public ApplicationBaseClass
{
public:
    Application(int &argc, char **argv);
    ~Application();
    FeedCore::Context *context();
    Settings *settings();
    void loadMainWindow();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    void activateMainWindow();
    void onLastWindowClosed();
    void onActivateRequested(const QStringList &arguments, const QString &workingDirectory);
    void bindContextPropertiesToSettings();
    void syncDefaultUpdateInterval();
    void syncExpireAge();
    void startNotifications();
};

#endif // APPLICATION_H
