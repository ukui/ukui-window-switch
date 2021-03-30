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

#include "ukws_workspace_box.h"

#include <QApplication>
#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMimeData>

#include "ukws_common.h"
#include "ukws_window_box.h"

UkwsWorkspaceBox::UkwsWorkspaceBox(QWidget *parent) : QWidget(parent)
{
    // 为缩略图控件注册监视对象
    installEventFilter(this);

    titleLabel = new UkwsWindowExtraLabel();
    thumbnailLabel = new UkwsWindowExtraLabel();
    closeButton = new QPushButton("X");

    mainLayout = new QVBoxLayout();
    topBarLayout = new QHBoxLayout();

    isSelected = false;

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    titleLabel->setSizePolicy(sizePolicy);
    sizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    closeButton->setSizePolicy(sizePolicy);

    // 设置控件最大尺寸
    titleLabel->setFixedHeight(32);
    titleLabel->setMinimumWidth(32);
    closeButton->setFixedSize(QSize(24, 24));
//    thumbnailLabel->setMaximumSize(QSize(640, 360));
    thumbnailLabel->setMinimumSize(QSize(1, 1));

    titleLabel->setContentsMargins(5, 0, 0, 0);
    closeButton->setContentsMargins(0, 0, 5, 0);
    thumbnailLabel->setContentsMargins(5, 5, 5, 5);

    /*
     * 设置控件布局（简单布局，直接手写）
     *
     *                 +------------+-------+   -╮
     * topBarLayout -> |    icon    | close |    |
     *                 +------------+-------+    |
     *                 |                    |    | -> mainLayout
     *                 |     thumbnail      |    |
     *                 |                    |    |
     *                 +--------------------+   -╯
     *
     */

    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignCenter);
    topBarLayout->setSpacing(0);
    topBarLayout->setAlignment(Qt::AlignJustify);

    topBarLayout->addWidget(titleLabel);
    topBarLayout->addWidget(closeButton);

    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(thumbnailLabel);
    this->setLayout(mainLayout);
    this->setObjectName(UKWS_OBJ_WSBOX);
    this->thumbnailLabel->setObjectName(UKWS_OBJ_WSBOX_THUMBNAIL);

    connect(closeButton, &QPushButton::released,
            this, &UkwsWorkspaceBox::onCloseButtonRealsed);

    closeButton->hide();

    this->installEventFilter(this);
    this->setAcceptDrops(true);
    this->setProperty("selected", false);
}

QString UkwsWorkspaceBox::getTitle()
{
    return this->title;
}

void UkwsWorkspaceBox::setTitle(QString title)
{
    this->title = title;
//    titleLabel->setText(this->title);
    this->updateTitleBySize();
}

void UkwsWorkspaceBox::updateTitleBySize()
{
    QFontMetrics fontMetrics(titleLabel->font());
    int fontSize = fontMetrics.width(this->title);
    QString formatTitle = this->title;
    if (fontSize > (titleLabel->width() - 5))
        formatTitle = fontMetrics.elidedText(this->title, Qt::ElideRight,
                                             titleLabel->width() - 10);

    titleLabel->setText(formatTitle);
}

WnckWorkspace *UkwsWorkspaceBox::getWnckWorkspace()
{
    return wnckWorkspace;
}

void UkwsWorkspaceBox::setWnckWorkspace(WnckWorkspace *workspace)
{
    wnckWorkspace = workspace;
}

void UkwsWorkspaceBox::setSizeByThumbnailSize(int width, int height)
{
    titleLabel->setFixedSize(width, 32);
    thumbnailLabel->setFixedSize(width, height);
    this->setFixedSize(width + 18, height + 32 + 18);
}

void UkwsWorkspaceBox::setBackground(QPixmap thumbnail)
{
    QSize size = thumbnailLabel->size();
    background = thumbnail.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    updateDesktopViewThumbnail(QPixmap(0, 0));
//    thumbnailLabel->setPixmap(background);
}

bool UkwsWorkspaceBox::eventFilter(QObject *object, QEvent *event)
{
    if (object == this) {
//        if (event->type() == QEvent::Enter) {
        if (event->type() == QEvent::MouseButtonPress) {
            emit doHover(this->index);

            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            return true;
        }

        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)
                emit selectedWorkspace(this->index);
            return true;
        }

//        if (event->type() == QEvent::Leave) {
//            return true;
//        }

        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent *>(event);
            QByteArray byteData = dragEvent->mimeData()->data("application/x-dnditemdata");
            QDataStream dataStream(&byteData, QIODevice::ReadOnly);
            QString wbTitle;
            int indIndex;
            int wbIndex;

            dataStream >> indIndex >> wbIndex >> wbTitle;
            dragEvent->accept();
        }

        if (event->type() == QEvent::DragLeave) {
            QDragLeaveEvent *dragLeave = static_cast<QDragLeaveEvent *>(event);

            dragLeave->accept();
        }

        if (event->type() == QEvent::Drop) {
            QDropEvent *dropEvent = static_cast<QDropEvent *>(event);

            QByteArray byteData = dropEvent->mimeData()->data("application/x-dnditemdata");
            QDataStream dataStream(&byteData, QIODevice::ReadOnly);
            QString wbTitle;
            int indIndex;
            int wbIndex;

            dataStream >> indIndex >> wbIndex >> wbTitle;
            if (indIndex != index) {
                emit windowChangeWorkspace(wbIndex, indIndex, index);
                dropEvent->accept();
            } else {
                dropEvent->ignore();
            }
        }
    }

    return false;
}

void UkwsWorkspaceBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

    QWidget::paintEvent(event);
}

void UkwsWorkspaceBox::onCloseButtonRealsed()
{

}

void UkwsWorkspaceBox::updateDesktopViewThumbnail(QPixmap viewPixmap)
{
    QPixmap tmpPixmap;
    QPainter painter;
    QSize size = thumbnailLabel->size();

    // 将桌面窗口视图叠加到背景上，形成桌面视图
    desktopViewPixmap = background;
    painter.begin((&desktopViewPixmap));
    if (!viewPixmap.isNull())
        tmpPixmap = viewPixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, size.width(), size.height(), tmpPixmap);
    painter.end();

    desktopViewDarkPixmap = desktopViewPixmap;
    painter.begin(&desktopViewDarkPixmap);
    painter.fillRect(desktopViewDarkPixmap.rect(), QColor(0, 0, 0, 159));
    painter.end();

    updateThumbnail();
}

void UkwsWorkspaceBox::setTitleStyle(QString style)
{
    titleLabel->setStyleSheet(style);
}

void UkwsWorkspaceBox::setBoxStyle(QString style)
{
    this->setStyleSheet(style);
}

void UkwsWorkspaceBox::setSelectStatus(bool status)
{
    isSelected = status;
    setProperty("selected", status);
    setStyle(QApplication::style());

    updateThumbnail();
}

void UkwsWorkspaceBox::updateThumbnail()
{
    if (isSelected)
        thumbnailLabel->setPixmap(desktopViewPixmap);
    else
        thumbnailLabel->setPixmap(desktopViewDarkPixmap);
}
