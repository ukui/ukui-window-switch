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

#ifndef UKWSWORKSPACEBOX_H
#define UKWSWORKSPACEBOX_H

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <xcb/xcb.h>

#include "ukws_window_extra_label.h"
#include "ukws_wnck_operator.h"

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QPixmap>
#include <QSize>
#include <QEvent>
#include <QPushButton>

class UkwsWorkspaceBox : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsWorkspaceBox(QWidget *parent = nullptr);

    QString getTitle();
    void setTitle(QString title);
    void updateTitleBySize();

    WnckWorkspace *getWnckWorkspace();
    void setWnckWorkspace(WnckWorkspace *workspace);

    void setSizeByThumbnailSize(int width, int height);
    void setBackground(QPixmap thumbnail);
    void setTitleStyle(QString style);
    void setBoxStyle(QString style);
    void setSelectStatus(bool status);
    void updateThumbnail();

    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *);

    int index;
    UkwsWnckOperator *wmOperator;

signals:
    void doHover(int index);
    void selectedWorkspace(int index);
    void windowChangeWorkspace(int wbIndex, int srcWsIndex, int dstWsIndex);

public slots:
    void onCloseButtonRealsed();
//    void activateWnckWorkspace();
    void updateDesktopViewThumbnail(QPixmap viewPixmap);

private:
    QString title;
    QPixmap desktopViewPixmap;
    QPixmap desktopViewDarkPixmap;
    QPixmap background;

    UkwsWindowExtraLabel *titleLabel;
    UkwsWindowExtraLabel *thumbnailLabel;
    QPushButton *closeButton;

    QVBoxLayout *mainLayout;
    QHBoxLayout *topBarLayout;

    WnckWorkspace *wnckWorkspace;

    bool isSelected;
};

#endif // UKWSWORKSPACEBOX_H
