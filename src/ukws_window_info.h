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

#ifndef UKWS_WINDOW_INFO_H
#define UKWS_WINDOW_INFO_H

extern "C" {
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
}

#include <QObject>
#include <QLabel>
#include <QPixmap>
#include <QRect>

class UkwsWindowInfo : public QObject
{
    Q_OBJECT
public:
    explicit UkwsWindowInfo(QObject *parent = nullptr);

    void setOrigPixmapByWnck();
    void setScaledPixmapByScale();
    void setScaledPixmapByScale(float scale);
    WnckWindow *getWnckWindow();
    void setWnckWindow(WnckWindow *window);

    QPixmap origPixmap;
    QPixmap scaledPixmap;

    unsigned long Xid;
    unsigned long frameXid;
    int winType;
    int winX;
    int winY;
    int winWidth;
    int winHeight;
    int winLeftOffset;
    int winRightOffset;
    int winTopOffset;
    int winBottomOffset;
    bool hasFrame;
    QRect winRect;

    float scale;

private:
    void fixFrameWindowArea();

    WnckWindow *wnckWin;
};

#endif // UKWS_WINDOW_INFO_H
