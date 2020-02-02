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
