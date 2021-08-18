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

#include "ukws_wayland_handler.h"
#include "qwindowdefs.h"

#include <QTime>
#include <QDebug>

UkwsWaylandHandler::UkwsWaylandHandler()
{
    wl_winIdList.clear();
}

void UkwsWaylandHandler::addWaylandWindow(quintptr wl_winId)
{
    wl_winIdList.append(wl_winId);
    qDebug() << "new wayland window: " << wl_winId;
}

void UkwsWaylandHandler::removeWaylandWindow(quintptr wl_winId)
{
    qDebug() << "remove wayland window: " << wl_winId;
    qDebug() << "The quantity of wayland window: " << wl_winIdList.size();
    for(int i = 0; i < wl_winIdList.size(); i++)
    {
        qDebug() << "wayland window: " << wl_winIdList.at(i);
        if(wl_winIdList.at(i) == wl_winId)
            wl_winIdList.removeAt(i);
    }
}

QString UkwsWaylandHandler::getPlatformStr()
{
    QString platformStr = getenv(XDG_TYPE);
    return platformStr;
}

void UkwsWaylandHandler::setWaylandWindowIconName(QString iconName)
{
    Q_UNUSED(iconName);
}

void UkwsWaylandHandler::setWaylandWindowTitle(QString title)
{
    Q_UNUSED(title);
}

void UkwsWaylandHandler::raiseWaylandWindow(quint32 wl_winId)
{
    Q_UNUSED(wl_winId);
}

void UkwsWaylandHandler::lowerWaylandWindow(quint32 wl_winId)
{
    Q_UNUSED(wl_winId);
}
