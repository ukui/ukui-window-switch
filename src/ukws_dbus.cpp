#include "ukws_dbus.h"

//#define KYLIN_PLUGIN_BUS_NAME       "org.gnome.Mutter.KylinPlugin"
//#define KYLIN_PLUGIN_OBJECT_PATH    "/org/gnome/Mutter/KylinPlugin"
//#define KYLIN_PLUGIN_INTERFACE      "org.gnome.Mutter.KylinPlugin"

#define KYLIN_PLUGIN_BUS_NAME       "org.ukui.Biometric"
#define KYLIN_PLUGIN_OBJECT_PATH    "/org/ukui/Biometric"
#define KYLIN_PLUGIN_INTERFACE      "org.ukui.Biometric"

#include <QDebug>

UkwsDbusWatcher::UkwsDbusWatcher()
{
    watcher = new QDBusServiceWatcher(KYLIN_PLUGIN_BUS_NAME,
                                      QDBusConnection::systemBus(),
                                      QDBusServiceWatcher::WatchForOwnerChange,
                                      this);
}

UkwsDbusWatcher::~UkwsDbusWatcher()
{

}

void UkwsDbusWatcher::start()
{
    QObject::connect(watcher,  &QDBusServiceWatcher::serviceOwnerChanged,
                     this, &UkwsDbusWatcher::onDBusNameOwnerChanged);
}

void UkwsDbusWatcher::stop()
{
    QObject::disconnect(watcher,  &QDBusServiceWatcher::serviceOwnerChanged,
                        this, &UkwsDbusWatcher::onDBusNameOwnerChanged);
}

void UkwsDbusWatcher::onDBusNameOwnerChanged(const QString &name,
                                            const QString &oldOwner,
                                            const QString &newOwner)
{
    Q_UNUSED(oldOwner);

    if (name == KYLIN_PLUGIN_BUS_NAME) {
        if (newOwner.isEmpty()) {
            qWarning() << "service status changed:" << "inactivate";
        } else {
            qWarning() << "service status changed:" << "activate";
        }

//        Q_EMIT serviceStatusChanged(!newOwner.isEmpty());
    }
}


UkwsDbusServer::UkwsDbusServer()
{

}

UkwsDbusClient::UkwsDbusClient()
{

}
