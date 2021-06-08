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
