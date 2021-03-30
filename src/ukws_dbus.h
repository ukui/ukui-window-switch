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
