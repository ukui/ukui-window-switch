#ifndef UKWS_HELPER_H
#define UKWS_HELPER_H

extern "C" {
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
}

#include <QImage>
#include <QPixmap>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class UkwsHelper
{
public:
    UkwsHelper();

    static QImage gdkPixbufToQImage(GdkPixbuf *pixbuf);
    static QPixmap gdkPixbufToQPixmap(GdkPixbuf *pixbuf);

    static QImage qImageFromXImage(XImage* ximage);
    static QPixmap qPixmapFromXImage(XImage* ximage);

    static QPixmap getThumbnailByXid(XID xid = 0);
};

#endif // UKWS_HELPER_H
