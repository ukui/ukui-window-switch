/*
 * Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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
