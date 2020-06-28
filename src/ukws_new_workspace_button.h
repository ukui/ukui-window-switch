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

#ifndef UKWS_NEW_WORKSPACE_BUTTON_H
#define UKWS_NEW_WORKSPACE_BUTTON_H

#include "ukws_window_extra_label.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QEvent>
#include <QDebug>

class UkwsNewWorkspaceButton : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsNewWorkspaceButton(QWidget *parent = nullptr);
//    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void setSize(int width, int height);
    void setSizeByButtonSize(int width, int height);
//    void updateContent();

signals:
    void addWorkspace();

private:
    QPainterPath drawBackgroundPath();
    QPixmap drawBackground(QPainterPath path, QColor color);
    UkwsWindowExtraLabel *title;
    UkwsWindowExtraLabel *button;

    QVBoxLayout *mainLayout;

    QPixmap normalPixmap;
    QPixmap hoverPixmap;
    QPixmap pressPixmap;
    QPixmap disablePixmap;
};

#endif // UKWS_NEW_WORKSPACE_BUTTON_H
