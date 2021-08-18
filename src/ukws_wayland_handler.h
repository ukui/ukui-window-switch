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

#ifndef UKWSWAYLANDHANDLER_H
#define UKWSWAYLANDHANDLER_H

#include <QList>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmavirtualdesktop.h>

#define XDG_TYPE "XDG_SESSION_TYPE"

class UkwsWaylandHandler
{
public:
    UkwsWaylandHandler();

    void addWaylandWindow(quintptr wl_winId);

    void removeWaylandWindow(quintptr wl_winId);

    void setWaylandWindowIconName(QString iconName);

    void setWaylandWindowTitle(QString title);

    QString getPlatformStr();

    QList<quint32> wl_winIdList;

    void raiseWaylandWindow(quint32 wl_winId);

    void lowerWaylandWindow(quint32 wl_winId);

    void getWaylandWindowList(quintptr wl_winId);

    QList<quintptr> waylandWindowList;


};

#endif // UKWSWAYLANDHANDLER_H
