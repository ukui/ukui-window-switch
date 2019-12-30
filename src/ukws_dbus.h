#ifndef UKWS_DBUS_H
#define UKWS_DBUS_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusServiceWatcher>

class UkwsDbusWatcher : public QObject
{
    Q_OBJECT
public:
    explicit UkwsDbusWatcher();
    ~UkwsDbusWatcher();

    void start();
    void stop();

    QDBusConnection::BusType busType;
    QString busName;
    QString objectPath;
    QDBusServiceWatcher *watcher;



private:
    void onDBusNameOwnerChanged(const QString &name,
                                const QString &oldOwner,
                                const QString &newOwner);

//    QDBusServiceWatcher *watcher;

};

class UkwsDbusServer
{
public:
    UkwsDbusServer();
};

class UkwsDbusClient
{
public:
    UkwsDbusClient();
};


#endif // UKWS_DBUS_H
