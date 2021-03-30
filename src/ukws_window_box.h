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

#ifndef UKWS_WINDOWBOX_H
#define UKWS_WINDOWBOX_H

extern "C" {
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
}

#include "ukws_window_extra_label.h"

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QPixmap>
#include <QSize>
#include <QEvent>
#include <QThread>
#include <QDrag>
#include <QTimer>
#include <QMimeData>

#define UKWS_WIDGET_SPACING         3
#define UKWS_ADDITIONAL_SPACING    10
#define UKWS_BORDER_WIDTH           2

#define UKWS_ICON_DEFAULT_SIZE      24
#define UKWS_ICON_DEFAULT_WIDTH     UKWS_ICON_DEFAULT_SIZE
#define UKWS_ICON_DEFAULT_HEIGHT    UKWS_ICON_DEFAULT_SIZE

#define UKWS_TITLE_DEFAULT_HEIGHT   UKWS_ICON_DEFAULT_HEIGHT

#define UKWS_THUMBNAIL_DEFAULT_WIDTH     320
#define UKWS_THUMBNAIL_DEFAULT_HEIGHT    180

#define UKWS_TITLE_WIDTH     (UKWS_THUMBNAIL_DEFAULT_WIDTH - UKWS_ICON_DEFAULT_WIDTH - UKWS_WIDGET_SPACING)
#define UKWS_TITLE_HEIGHT    UKWS_ICON_DEFAULT_HEIGHT

#define UKWS_WINDOWBOX_WIDTH     (UKWS_THUMBNAIL_DEFAULT_WIDTH + UKWS_BORDER_WIDTH * 2)
#define UKWS_WINDOWBOX_HEIGHT    (UKWS_ICON_DEFAULT_HEIGHT + UKWS_THUMBNAIL_DEFAULT_HEIGHT + UKWS_WIDGET_SPACING + UKWS_ADDITIONAL_SPACING)

// 外边距8，外边框4，边框紧贴图片，宽占用：(8 + 4) * 2 = 24，高占用：8 + 4 = 12
#define UKWS_THUMBNAIL_MARGIN   8
#define UKWS_THUMBNAIL_RADIUS   6
#define UKWS_WINDOWBOX_PADDING  5
#define UKWS_WINDOWBOX_BORDER   4
//#define UKWS_WINDOWBOX_RADIUS   6

#define UKWS_DRAG_SCALE_INTERVAL_MS 125
#define UKWS_DRAG_SCALE_TIMES       4

class UkwsWindowBox : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsWindowBox(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);

    void setSubWidgetSizeByThnSize(int w, int h);
    void setWinboxSizeByHeight(int height);

    QString getTitle();
    void setTitle(QString title);
    void updateTitleBySize();

    WnckWindow *getWnckWindow();
    void setWnckWindow(WnckWindow *window);

    void setIcon(QPixmap icon);
    void setIconByWnck();
    void fixFrameWindowArea();
    void setOrigThumbnailByWnck();
    void updateThumbnail();
    void setThumbnail(QPixmap origPixmap);
    void setThumbnailByWnck();
    void setDragIconSize(QSize size);
    QPixmap windowPixmap();

    void setThumbnailHover();
    void setThumbnailNormal();
    void setWindowBoxSelected();
    void setWindowBoxUnselected();
    void setTitleAutoHide(bool autoHide);

    void moveToWorkspace(int wsIndex);

    bool windowIsAlive();

    bool eventFilter(QObject *watched, QEvent *event);

    unsigned long frameXid;
    int winX;
    int winY;
    int winWidth;
    int winHeight;
    int winLeftOffset;
    int winRightOffset;
    int winTopOffset;
    int winBottomOffset;

    int index;
    int parentIndex;
    int winType;
    bool dragable;
    bool isDragged;

    QSize iconSize;
    QSize titleSize;
    QSize thnRealSize;
    QSize winboxSize;
    QRect windowRect;
    QPoint thumbnailOffset;

    Qt::TransformationMode iconTransformationMode;
    Qt::TransformationMode thumbnailTransformationMode;

signals:
    void clicked(UkwsWindowBox *winbox);
    void closeBtnClicked(UkwsWindowBox *winbox);

public slots:
    void activateWnckWindow();
    void closeWnckWindow();

protected:
    void leaveEvent(QEvent *);

private:
    QPixmap makeRadiusPixmap(QPixmap orig, int radius);
    void scaleDragPixmap();

    QString title;

    UkwsWindowExtraLabel *iconLabel;
    UkwsWindowExtraLabel *titleLabel;
    UkwsWindowExtraLabel *closeLabel;
    UkwsWindowExtraLabel *thumbnailLabel;

    QPixmap thnUnselectedPixmap;
    QPixmap thnSelectedPixmap;

    QVBoxLayout *mainLayout;
    QHBoxLayout *topBarLayout;

    WnckWindow *wnckWin;
    bool hasFrame;

    bool isSelected;

    QDrag *drag;
    QSize dragIconSize;
    QTimer scaleTimer;
    int scaleTimes;
    QSize scaleUnitSize;

    bool titleAutoHide;
};

#endif // UKWS_WINDOWBOX_H
