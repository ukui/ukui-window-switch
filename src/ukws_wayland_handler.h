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
