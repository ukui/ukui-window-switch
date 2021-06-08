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

#ifndef UKWS_HELPER_H
#define UKWS_HELPER_H

extern "C" {
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
}

#include <QImage>
#include <QPixmap>
#include <QDebug>
#include <QAction>

namespace UkwsHelperXHeader {
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
}

using UkwsHelperXHeader::XImage;
using UkwsHelperXHeader::XID;

class UkwsHelper
{
public:
    UkwsHelper();

    static QImage gdkPixbufToQImage(GdkPixbuf *pixbuf);
    static QPixmap gdkPixbufToQPixmap(GdkPixbuf *pixbuf);

    static QImage qImageFromXImage(XImage* ximage);
    static QPixmap qPixmapFromXImage(XImage* ximage);

    static QPixmap getThumbnailByXid(XID xid = 0, int offsetLeft = 0, int offsetRight = 0,
                                     int offsetTop = 0, int offsetBottom = 0);

    static XID getParentWindowId(XID xid);
};

#endif // UKWS_HELPER_H
