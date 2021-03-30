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

#include "ukws_common.h"
#include "ukws_new_workspace_button.h"

#include <QPainter>
#include <QPainterPath>

UkwsNewWorkspaceButton::UkwsNewWorkspaceButton(QWidget *parent) : QWidget(parent)
{

    title = new UkwsWindowExtraLabel;
    button = new UkwsWindowExtraLabel;

    mainLayout = new QVBoxLayout;

    title->setObjectName(UKWS_OBJ_NEW_WS_TITLE);
    button->setObjectName(UKWS_OBJ_NEW_WS_LABEL);
    title->setContentsMargins(5, 0, 5, 0);
    button->setContentsMargins(5, 5, 5, 5);

    mainLayout->addWidget(title);
    mainLayout->addWidget(button);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setMargin(0);

    this->setLayout(mainLayout);
    this->installEventFilter(this);
}

void UkwsNewWorkspaceButton::setSize(int width, int height)
{
    setSizeByButtonSize(width - 18, height - 32 - 18);
}

void UkwsNewWorkspaceButton::setSizeByButtonSize(int width, int height)
{
    title->setFixedSize(width, 32);
    button->setFixedSize(width, height);
    this->setFixedSize(width + 18, height + 32 + 18);

    // 绘制背景
    QPainterPath path = drawBackgroundPath();
    normalPixmap = drawBackground(path, QColor(63, 63, 63, 63));
    hoverPixmap = drawBackground(path, QColor(95, 95, 95, 63));

    title->setText("新建工作区");
    button->setPixmap(normalPixmap);
}

bool UkwsNewWorkspaceButton::eventFilter(QObject *object, QEvent *event)
{
    if (object == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            qDebug() << "newWorkspaceLabel MouseButtonPress.";
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            qDebug() << "newWorkspaceLabel MouseButtonRelease.";
        }

        if (event->type() == QEvent::Enter) {
            button->setPixmap(hoverPixmap);
            qDebug() << "newWorkspaceLabel Enter.";
        }

        if (event->type() == QEvent::Leave) {
            button->setPixmap(normalPixmap);
            qDebug() << "newWorkspaceLabel Leave.";
        }

        return true;
    }

    return false;
}

QPainterPath UkwsNewWorkspaceButton::drawBackgroundPath()
{
    int width = button->width();
    int height = button->height();
    int w, h, x, y, r;
    QPainterPath path;


    // 图形宽度
    r = (4 + height / 100) / 2;

    // 绘制设置竖矩形
    h = height / 3;
    x = width / 2 - r;
    path.addRect(x, h, 2 * r, h);

    // 绘制设置横矩形
    w = h;
    x = (width - w) / 2;
    y = (height / 2) - r;
    path.addRect(x, y, w, 2 * r);

    // 设置重叠区域策略
    path.setFillRule(Qt::OddEvenFill);

    return path;
}

QPixmap UkwsNewWorkspaceButton::drawBackground(QPainterPath path, QColor color)
{
    int width = button->width();
    int height = button->height();
    QPainter painter;
    QPixmap pixmap = QPixmap(width, height);

    pixmap.fill(Qt::transparent);
    painter.begin(&pixmap);
    painter.fillRect(0, 0, width, height, color);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(QBrush(QColor(255, 255, 255)));
//    color.setAlpha(255);
    painter.setPen(QPen(color));
    painter.drawPath(path);
    painter.end();

    return pixmap;
}
