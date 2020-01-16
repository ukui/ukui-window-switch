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

#include "ukws_window_box.h"

#include <QString>
#include <QDate>
#include <QSize>
#include <QEvent>
#include <QResizeEvent>

#include <QDebug>
#include <QStyleOption>
#include <QPainter>

#include "ukws_common.h"
#include "ukws_helper.h"

using namespace UkwsHelperXHeader;

#include <QX11Info>

static const QSize UKWS_TITLE_SIZE = QSize(UKWS_TITLE_WIDTH, UKWS_TITLE_HEIGHT);
static const QSize UKWS_ICON_SIZE = QSize(UKWS_ICON_DEFAULT_WIDTH, UKWS_ICON_DEFAULT_HEIGHT);
static const QSize UKWS_THUMBNAIL_SIZE = QSize(UKWS_THUMBNAIL_DEFAULT_WIDTH, UKWS_THUMBNAIL_DEFAULT_HEIGHT);

UkwsWindowBox::UkwsWindowBox(QWidget *parent) : QWidget(parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground, true);
    dragable = false;
    winLeftOffset = 0;
    winRightOffset = 0;
    winTopOffset = 0;
    winBottomOffset = 0;
    frameXid = 0;
    hasFrame = false;

    titleLabel = new UkwsWindowExtraLabel();
    iconLabel = new UkwsWindowExtraLabel();
    thumbnailLabel = new UkwsWindowExtraLabel();

    titleSize = UKWS_TITLE_SIZE;
    iconSize = UKWS_ICON_SIZE;
    winboxSize = QSize(UKWS_WINDOWBOX_WIDTH, UKWS_WINDOWBOX_HEIGHT);

    iconLabel->setAlignment(Qt::AlignCenter);
    thumbnailLabel->setAlignment(Qt::AlignCenter);

    iconTransformationMode = Qt::SmoothTransformation;
    thumbnailTransformationMode = Qt::SmoothTransformation;

    mainLayout = new QVBoxLayout();
    topBarLayout = new QHBoxLayout();

    wnckWin = NULL;
    windowRect.setRect(0, 0, 0, 0);

    titleLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    // 自动缩放图片
    //	titleLabel->setScaledContents(true);
    iconLabel->setScaledContents(false);
    thumbnailLabel->setScaledContents(false);

    // 设置控件缩放方式
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    sizePolicy.setHorizontalStretch(0);
//    sizePolicy.setVerticalStretch(0);
    iconLabel->setSizePolicy(sizePolicy);
    sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    titleLabel->setSizePolicy(sizePolicy);
    sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);

    // 设置控件最大尺寸
    iconLabel->resize(QSize(32, 32));
    titleLabel->setFixedHeight(32);
    titleLabel->setMinimumWidth(1);
    thumbnailLabel->setMinimumSize(QSize(1, 1));

//    iconLabel->setContentsMargins(5, 0, 0, 0);
    titleLabel->setContentsMargins(0, 0, 5, 0);
//    thumbnailLabel->setContentsMargins(3, 3, 3, 3);

    /*
     * 设置控件布局（简单布局，直接手写）
     *
     *                 +------+-------+   -╮
     * topBarLayout -> | icon | title |    |
     *                 +------+-------+    |
     *                 |              |    | -> mainLayout
     *                 |  thumbnail   |    |
     *                 |              |    |
     *                 +--------------+   -╯
     *
     */

    mainLayout->setSpacing(UKWS_WIDGET_SPACING);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    topBarLayout->setSpacing(UKWS_WIDGET_SPACING);
    topBarLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    topBarLayout->addWidget(iconLabel);
    topBarLayout->addWidget(titleLabel);

    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(thumbnailLabel);
    this->setLayout(mainLayout);

    // 为缩略图控件注册监视对象
    thumbnailLabel->installEventFilter(this);

    // 设置缩略图控件的objectName
    titleLabel->setObjectName(UKWS_OBJ_WINBOX_WIN_NAME);
    thumbnailLabel->setObjectName(UKWS_OBJ_WINBOX_THUMBNAIL);
    this->setObjectName(UKWS_OBJ_WINBOX);

    mainLayout->setContentsMargins(5, 5, 5, 5);
    setThumbnailNormal();

//    connect(this, &UkwsWindowBox::clicked, this, &UkwsWindowBox::activateWnckWindow);

    this->setAttribute(Qt::WA_TranslucentBackground, true);
    titleLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    iconLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    thumbnailLabel->setAttribute(Qt::WA_TranslucentBackground, true);

    this->setWindowBoxUnselected();
}

void UkwsWindowBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

    QWidget::paintEvent(event);
}

void UkwsWindowBox::setSubWidgetSize(int w, int h)
{
    // winbox宽w，高h，外框2；
    // icon宽高32，上下间距0，左间距5（缩略图的左边框2，保持对齐），边框0；
    // title左边距0，右边距5（缩略图右边框2，保持对齐），边框0；
    // 调整值：2x2 + 5 + 32 + 5 = 46
    titleLabel->setFixedSize(w - 46, 32);
    this->updateTitleBySize();

    // winbox宽w，高h，外框2；
    // icon宽高32，上下间距0
    // 缩略图，外边距3；
    // 调整值：宽，2x2 + 3x2 = 10，高，2x2 + 32 + 3x2 = 42
    thumbnailLabel->setFixedSize(w - 10, h - 42);
}

void UkwsWindowBox::setWinboxSizeByHeight(int height)
{
    // winbox边框2；
    // icon，高32，上下间距0，上下边框0；
    // 缩略图，外边距3，外边框2，图片间隔2；
    // 调整值：高，2x2 + 32 + 3x2 + 2x2 + 2x2 = 50
    int thnHeight = height - 50;

    int w, h;
    float scale;
    float maxScale = 20.0 / 9;
    int maxWidth = int(thnHeight * maxScale);
    w = thumbnailLabel->originalQPixmap.size().width();
    h = thumbnailLabel->originalQPixmap.size().height();

    // 计算缩放因子
    if (h > thnHeight)
        scale = (float)thnHeight / h;
    else
        scale = 1;

    // 限制最大宽度，调节缩放因子
    if (w * scale > maxWidth)
        scale = (float)maxWidth / w;

    // 计算缩略图大小
    w = int(w * scale);
    h = int(h * scale);

    // winbox边框2；
    // icon高32，上下间距0，上下边框0；
    // 缩略图宽w高h，外边距3，外边框2，图片间隔2；
    // 调整值：宽，2x2 + 3x2 + 2x2 + 2x2 = 18，高，2x2 + 32 + 3x2 + 2x2 + 2x2 = 50
    this->setSubWidgetSize(w + 18, h + 48);
    this->setFixedSize(w + 18, height);
}

WnckWindow *UkwsWindowBox::getWnckWindow()
{
    return wnckWin;
}
void UkwsWindowBox::setWnckWindow(WnckWindow *window)
{
    int x, y, w, h;
    wnckWin = window;
    wnck_window_get_client_window_geometry(window, &x, &y, &w, &h);
    windowRect.setRect(x, y, w, h);
}

void UkwsWindowBox::setTitle(QString title)
{
    this->title = title;
//    titleLabel->setText(this->title);
    this->updateTitleBySize();
}

void UkwsWindowBox::updateTitleBySize()
{
    QFontMetrics fontMetrics(titleLabel->font());
    int fontSize = fontMetrics.width(this->title);
    QString formatTitle = this->title;
    if (fontSize > (titleLabel->width() - 5)) {
        formatTitle = fontMetrics.elidedText(this->title, Qt::ElideRight,
                                             titleLabel->width() - 15);
    }

    titleLabel->setText(formatTitle);
}

void UkwsWindowBox::setIcon(QPixmap icon)
{
    iconLabel->originalQPixmap = icon;
    iconLabel->setPixmap(iconLabel->originalQPixmap
                         .scaled(iconSize.width() - 0, iconSize.height() - 0,
                                 Qt::KeepAspectRatio, iconTransformationMode));
}

void UkwsWindowBox::setIconByWnck()
{
    iconLabel->originalQPixmap = UkwsHelper::gdkPixbufToQPixmap(wnck_window_get_icon(wnckWin));
    iconLabel->setPixmap(iconLabel->originalQPixmap
                         .scaled(iconSize.width() - 0, iconSize.height() - 0,
                                 Qt::KeepAspectRatio, thumbnailTransformationMode));
}

void UkwsWindowBox::fixFrameWindowArea()
{
    // 判断窗口是否被窗口管理器添加了装饰窗口frame window
    int frameX, frameY, frameWidth, frameHeight;
    int origX, origY, origWidth, origHeight;

    wnck_window_get_geometry(wnckWin, &frameX, &frameY, &frameWidth, &frameHeight);
    wnck_window_get_client_window_geometry(wnckWin, &origX, &origY, &origWidth, &origHeight);

    winX = frameX;
    winY = frameY;
    winWidth = frameWidth;
    winHeight = frameHeight;

    if ((origWidth == frameWidth) && (origHeight == frameHeight)) {
        // 无窗口装饰区
        winLeftOffset = 0;
        winRightOffset = 0;
        winTopOffset = 0;
        winBottomOffset = 0;
        frameXid = wnck_window_get_xid(wnckWin);
        hasFrame = false;
    } else {
        // 有窗口装饰区，获取装饰区
        XWindowAttributes attr;
        Display *display = QX11Info::display();
        XID parentXid = UkwsHelper::getParentWindowId(wnck_window_get_xid(wnckWin));

        if (parentXid == ~(unsigned long)0) {
            // 获取父窗口ID失败，使用自身的WID作为frame窗口的ID，偏移量全为0
            winX = origX;
            winY = origY;
            winWidth = origWidth;
            winHeight = origHeight;
            winLeftOffset = 0;
            winRightOffset = 0;
            winTopOffset = 0;
            winBottomOffset = 0;
            frameXid = wnck_window_get_xid(wnckWin);
            hasFrame = false;

            return;
        } else {
            // frame窗口为本窗口的父窗口
            frameXid = parentXid;
        }

        // 获取frame窗口属性
        XGetWindowAttributes(display, frameXid, &attr);

        // 过滤窗口阴影
        winLeftOffset = frameX - attr.x;
        winRightOffset = attr.width - winLeftOffset - frameWidth;
        winTopOffset = frameY - attr.y;
        winBottomOffset = attr.height - winTopOffset - frameHeight;

        // 边界值修正
        if (winLeftOffset < 0) winLeftOffset = 0;
        if (winRightOffset < 0) winRightOffset = 0;
        if (winTopOffset < 0) winTopOffset = 0;
        if (winBottomOffset < 0) winBottomOffset = 0;

        hasFrame = true;
    }
}

void UkwsWindowBox::setOrigThumbnailByWnck()
{
    fixFrameWindowArea();
    thumbnailLabel->originalQPixmap = UkwsHelper::getThumbnailByXid(frameXid,
                                                                    winLeftOffset,
                                                                    winRightOffset,
                                                                    winTopOffset,
                                                                    winBottomOffset);
}

void UkwsWindowBox::setThumbnail(QPixmap origPixmap)
{
    QSize labelSize = thumbnailLabel->size();
    thumbnailLabel->originalQPixmap = origPixmap;

    // 缩略图，外边框2，图片间距2；
    // 调整值：宽，2x2 + 2x2 = 8，高，2x2 + 2x2 = 8
    scaledThumbnail = origPixmap.scaled(labelSize.width() - 8,
                                        labelSize.height() - 8,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);

    QSize thumbnailSize = scaledThumbnail.size();
    QSize size = (thumbnailLabel->contentsRect().size() - thumbnailSize) / 2;
    thumbnailOffset = QPoint(size.width(), size.height());
    thumbnailLabel->setPixmap(scaledThumbnail);
}

void UkwsWindowBox::setThumbnailByWnck()
{
    fixFrameWindowArea();
    QSize labelSize = thumbnailLabel->size();
    if (labelSize.width() == 0 || labelSize.height() == 0)
        thumbnailLabel->originalQPixmap = UkwsHelper::getThumbnailByXid(frameXid,
                                                                        winLeftOffset,
                                                                        winRightOffset,
                                                                        winTopOffset,
                                                                        winBottomOffset);

    // 缩略图，外边框2，图片间距2；
    // 调整值：宽，2x2 + 2x2 = 8，高，2x2 + 2x2 = 8
    scaledThumbnail = thumbnailLabel->originalQPixmap.scaled(labelSize.width() - 8,
                                                             labelSize.height() - 8,
                                                             Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation);

    QSize thumbnailSize = scaledThumbnail.size();
    QSize size = (thumbnailLabel->contentsRect().size() - thumbnailSize) / 2;
    thumbnailOffset = QPoint(size.width(), size.height());
    thumbnailLabel->setPixmap(scaledThumbnail);
}

QPixmap UkwsWindowBox::windowPixmap()
{
    return thumbnailLabel->originalQPixmap;
}

QString UkwsWindowBox::getTitle()
{
    return this->title;
}

void UkwsWindowBox::setThumbnailHover()
{

}

void UkwsWindowBox::setThumbnailNormal()
{

}

void UkwsWindowBox::setWindowBoxSelected()
{
    this->setStyleSheet("QWidget#winbox{padding:0px; border:2px solid rgb(255, 255, 255, 255);}");
}

void UkwsWindowBox::setWindowBoxUnselected()
{
    this->setStyleSheet("QWidget#winbox{padding:0px; border:2px solid rgb(255, 255, 255, 0);}");
}

void UkwsWindowBox::moveToWorkspace(int wsIndex)
{
    WnckScreen *screen = wnck_window_get_screen(wnckWin);
    WnckWorkspace *workspace = wnck_screen_get_workspace(screen, wsIndex);
    wnck_window_move_to_workspace(wnckWin, workspace);
}

bool UkwsWindowBox::eventFilter(QObject *watched, QEvent *event)
{
    static bool pressed = 0;

    if (watched == thumbnailLabel) {
        if (event->type() == QEvent::Enter) {
            pressed = false;
            setThumbnailHover();

            return true;
        }

        if (event->type() == QEvent::Leave) {
            pressed = false;
            setThumbnailNormal();

            return true;
        }

        // 左键按下则标记按下状态
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton)
                pressed = true;

            return true;
        }

        // 在同一个WindowBox中按下鼠标并抬起才算选定
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (pressed && (mouseEvent->button() == Qt::LeftButton)) {
                pressed = false;
                emit clicked(this);

                return true;
            }
        }

        if (event->type() == QEvent::MouseMove) {
            if (!pressed)
                return false;

            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            // 禁用拖拽，直接返回
            if (!dragable)
                return true;

            // 缩略图置灰
            QPixmap tempPixmap = scaledThumbnail;
            QPainter painter;
            painter.begin(&tempPixmap);
            painter.fillRect(scaledThumbnail.rect(), QColor(0, 0, 0, 127));
            painter.end();
            thumbnailLabel->setPixmap(tempPixmap);

            // 拖拽处理
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            QByteArray byteData;
            QDataStream dataStream(&byteData, QIODevice::WriteOnly);

            dataStream << parentIndex << index << title;
            mimeData->setData("application/x-dnditemdata", byteData);
            drag->setMimeData(mimeData);
            drag->setPixmap(scaledThumbnail);
//            drag->setHotSpot(mouseEvent->pos() - winbox->pos() + QPoint(0 - 14, 0 + 32 - 14));

            qDebug() << "----------------";
            qDebug() << thumbnailLabel->size() - scaledThumbnail.size();
            qDebug() << thumbnailLabel->contentsRect().size() - scaledThumbnail.size();
            qDebug() << thumbnailOffset;
            qDebug() << thumbnailLabel->size();
            qDebug() << thumbnailLabel->contentsRect().size();

            drag->setHotSpot(mouseEvent->pos() - thumbnailOffset);

            if (drag->exec()) {
                qDebug() << "Drag Done";
                thumbnailLabel->setPixmap(scaledThumbnail);
            } else {
                qDebug() << "Drag Cancel";
                thumbnailLabel->setPixmap(scaledThumbnail);
            }

            drag->deleteLater();

            return true;
        }
    }

    return false;
}

void UkwsWindowBox::activateWnckWindow()
{
    WnckWorkspace *workspace = wnck_window_get_workspace(wnckWin);
    unsigned long timestamp = QX11Info::getTimestamp();

    wnck_workspace_activate(workspace, timestamp);
    wnck_window_activate(wnckWin, timestamp);
}
