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

#include "ukws_window_info.h"

#include "ukws_common.h"
#include "ukws_helper.h"

using namespace UkwsHelperXHeader;

#include <QX11Info>

UkwsWindowInfo::UkwsWindowInfo(QObject *parent) : QObject(parent)
{
    Xid = -1;
    frameXid = -1;
    winType = -1;
    winX = 0;
    winY = 0;
    winWidth = 0;
    winHeight = 0;
    winLeftOffset = 0;
    winRightOffset = 0;
    winTopOffset = 0;
    winBottomOffset = 0;
    hasFrame = false;

    wl_windowIndex = -1;

}

void UkwsWindowInfo::fixFrameWindowArea()
{
    // 判断窗口是否被窗口管理器添加了装饰窗口frame window
    int frameX, frameY, frameWidth, frameHeight;
    int origX, origY, origWidth, origHeight;

    wnck_window_get_geometry(wnckWin, &frameX, &frameY, &frameWidth, &frameHeight);
    wnck_window_get_client_window_geometry(wnckWin, &origX, &origY, &origWidth, &origHeight);

    winX = frameX;
    winY = frameY;
    winWidth = frameWidth;
    winHeight = frameHeight;

    if ((origWidth == frameWidth) && (origHeight == frameHeight)) {
        // 无窗口装饰区
        winLeftOffset = 0;
        winRightOffset = 0;
        winTopOffset = 0;
        winBottomOffset = 0;
        frameXid = wnck_window_get_xid(wnckWin);
        hasFrame = false;
    } else {
        // 有窗口装饰区，获取装饰区
        XWindowAttributes attr;
        Display *display = QX11Info::display();
        XID parentXid = UkwsHelper::getParentWindowId(wnck_window_get_xid(wnckWin));

        if (parentXid == ~(unsigned long)0) {
            // 获取父窗口ID失败，使用自身的WID作为frame窗口的ID，偏移量全为0
            winX = origX;
            winY = origY;
            winWidth = origWidth;
            winHeight = origHeight;
            winLeftOffset = 0;
            winRightOffset = 0;
            winTopOffset = 0;
            winBottomOffset = 0;
            frameXid = wnck_window_get_xid(wnckWin);
            hasFrame = false;

            return;
        } else {
            // frame窗口为本窗口的父窗口
            frameXid = parentXid;
        }

        // 获取frame窗口属性
        XGetWindowAttributes(display, frameXid, &attr);

        // 过滤窗口阴影
        winLeftOffset = frameX - attr.x;
        winRightOffset = attr.width - winLeftOffset - frameWidth;
        winTopOffset = frameY - attr.y;
        winBottomOffset = attr.height - winTopOffset - frameHeight;

        // 边界值修正
        if (winLeftOffset < 0) winLeftOffset = 0;
        if (winRightOffset < 0) winRightOffset = 0;
        if (winTopOffset < 0) winTopOffset = 0;
        if (winBottomOffset < 0) winBottomOffset = 0;

        hasFrame = true;
    }
}

void UkwsWindowInfo::setOrigPixmapByWnck()
{
    fixFrameWindowArea();
    origPixmap = UkwsHelper::getThumbnailByXid(frameXid, winLeftOffset,
                                               winRightOffset, winTopOffset,
                                               winBottomOffset);
}

void UkwsWindowInfo::setOrigPixmapByWayland()
{
    QPixmap thumbnail;
    thumbnail = QPixmap(800, 600);
    thumbnail.fill(QColor(0, 0, 0, 127));
    origPixmap = thumbnail;
}

void UkwsWindowInfo::setScaledPixmapByScale()
{
    int width = int(origPixmap.width() * scale);
    scaledPixmap = origPixmap.scaledToWidth(width, Qt::SmoothTransformation);
}

void UkwsWindowInfo::setScaledPixmapByScale(float scale)
{
    int width = int(origPixmap.width() * scale);
    this->scale = scale;
    scaledPixmap = origPixmap.scaledToWidth(width, Qt::SmoothTransformation);
}

WnckWindow *UkwsWindowInfo::getWnckWindow()
{
    return wnckWin;
}

void UkwsWindowInfo::setWnckWindow(WnckWindow *window)
{
    int x, y, w, h;

    wnckWin = window;
    wnck_window_get_client_window_geometry(window, &x, &y, &w, &h);
    winType = wnck_window_get_window_type(window);
    winRect.setRect(x, y, w, h);
    //fixFrameWindowArea();
}

void UkwsWindowInfo::setWaylandWindow(quint32 waylandId)
{
    wl_windowIndex = waylandId;
}
