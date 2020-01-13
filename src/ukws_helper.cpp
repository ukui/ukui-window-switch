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


#include "ukws_helper.h"

using namespace UkwsHelperXHeader;

#include <QX11Info>

UkwsHelper::UkwsHelper()
{

}

QImage UkwsHelper::gdkPixbufToQImage(GdkPixbuf *pixbuf)
{
    gchar *buffer;
    gsize buffer_size;
    GError *error = 0;
    gboolean b;

    b = gdk_pixbuf_save_to_buffer(pixbuf, &buffer, &buffer_size, "png", &error, NULL);
    if (!b) {
        Q_ASSERT(error);
        qWarning("GdkPixbuf到QImage装换失败：%s", error->message);
        g_error_free(error);

        return QImage();
    } else {
        const QByteArray data = QByteArray::fromRawData(buffer, buffer_size);
        QImage image;

        image.loadFromData(data);
        g_free(buffer);

        return image;
    }
}

QPixmap UkwsHelper::gdkPixbufToQPixmap(GdkPixbuf *pixbuf)
{
    return QPixmap::fromImage(gdkPixbufToQImage(pixbuf));
}

QImage UkwsHelper::qImageFromXImage(XImage* ximage)
{
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    if (ximage->depth == 24)
        format = QImage::Format_RGB32;
    else if (ximage->depth == 16)
        format = QImage::Format_RGB16;

    QImage image = QImage(reinterpret_cast<uchar*>(ximage->data),
                          ximage->width, ximage->height,
                          ximage->bytes_per_line, format).copy();

    // 大端还是小端?
    if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && ximage->byte_order == MSBFirst)
            || (QSysInfo::ByteOrder == QSysInfo::BigEndian && ximage->byte_order == LSBFirst)) {

        for (int i = 0; i < image.height(); i++) {
            if (ximage->depth == 16) {
                ushort* p = reinterpret_cast<ushort*>(image.scanLine(i));
                ushort* end = p + image.width();
                while (p < end) {
                    *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                    p++;
                }
            } else {
                uint* p = reinterpret_cast<uint*>(image.scanLine(i));
                uint* end = p + image.width();
                while (p < end) {
                    *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000)
                         | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                    p++;
                }
            }
        }
    }

    // 修复alpha通道
    if (format == QImage::Format_RGB32) {
        QRgb* p = reinterpret_cast<QRgb*>(image.bits());
        for (int y = 0; y < ximage->height; ++y) {
            for (int x = 0; x < ximage->width; ++x)
                p[x] |= 0xff000000;
            p += ximage->bytes_per_line / 4;
        }
    }

    return image;
}

QPixmap UkwsHelper::qPixmapFromXImage(XImage* ximage)
{
    return QPixmap::fromImage(qImageFromXImage(ximage));
}

QPixmap UkwsHelper::getThumbnailByXid(XID xid)
{
    XWindowAttributes attr;
    XID parentXid = getParentWindowId(xid);
    Display *display = QX11Info::display();
    int ukwmShadowWidth = 26;
    int ukwmShadowTopOffset = 3;

    if (parentXid != ~(unsigned long)0)
        xid = parentXid;

    XGetWindowAttributes(display, xid, &attr);
    XImage *image = XGetImage(display, xid, ukwmShadowWidth,
                              ukwmShadowWidth - ukwmShadowTopOffset,
                              attr.width - ukwmShadowWidth * 2,
                              attr.height - ukwmShadowWidth * 2,
                              0xffffffff, ZPixmap);

    QPixmap thumbnail;
    if (image != nullptr) {
        thumbnail = qPixmapFromXImage(image);
        XDestroyImage(image);
    } else {
        // 无法获取到图像时，使用黑色半透明图像替代
        thumbnail = QPixmap(attr.width, attr.height);
        thumbnail.fill(QColor(0, 0, 0, 127));
    }

    return thumbnail;
}

XID UkwsHelper::getParentWindowId(XID xid)
{
    XID root, parent, *children = NULL;
    unsigned int childrenNum;

    if (!XQueryTree(QX11Info::display(), xid, &root, &parent, &children, &childrenNum))
        return ~0;

    if (children)
        XFree((char *)children);

    return parent;
}
