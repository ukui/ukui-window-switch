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
#include <QSizePolicy>
#include <QEvent>
#include <QResizeEvent>

#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QCoreApplication>

#include "ukws_common.h"
#include "ukws_helper.h"

using namespace UkwsHelperXHeader;

#include <QX11Info>

#define NO_ADD_FRAME

static const QSize UKWS_TITLE_SIZE = QSize(UKWS_TITLE_WIDTH, UKWS_TITLE_HEIGHT);
static const QSize UKWS_ICON_SIZE = QSize(UKWS_ICON_DEFAULT_WIDTH, UKWS_ICON_DEFAULT_HEIGHT);
static const QSize UKWS_THUMBNAIL_SIZE = QSize(UKWS_THUMBNAIL_DEFAULT_WIDTH, UKWS_THUMBNAIL_DEFAULT_HEIGHT);

UkwsWindowBox::UkwsWindowBox(QWidget *parent) : QWidget(parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground, true);
    dragable = false;
    isDragged = false;
    winLeftOffset = 0;
    winRightOffset = 0;
    winTopOffset = 0;
    winBottomOffset = 0;
    frameXid = 0;
    hasFrame = false;
    drag = nullptr;
    isSelected = false;
    titleAutoHide = false;

    iconLabel = new UkwsWindowExtraLabel();
    titleLabel = new UkwsWindowExtraLabel();
    closeLabel = new UkwsWindowExtraLabel();
    thumbnailLabel = new UkwsWindowExtraLabel();

    titleSize = UKWS_TITLE_SIZE;
    iconSize = UKWS_ICON_SIZE;
    winboxSize = QSize(UKWS_WINDOWBOX_WIDTH, UKWS_WINDOWBOX_HEIGHT);
    dragIconSize = QSize(0, 0);

    iconLabel->setAlignment(Qt::AlignCenter);
    thumbnailLabel->setAlignment(Qt::AlignCenter);
    closeLabel->setAlignment(Qt::AlignCenter);

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
    closeLabel->setScaledContents(false);

    // 设置控件缩放方式
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setRetainSizeWhenHidden(true);
    iconLabel->setSizePolicy(sizePolicy);
    closeLabel->setSizePolicy(sizePolicy);
    sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    titleLabel->setSizePolicy(sizePolicy);
    sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);

    // 设置控件最大尺寸
    iconLabel->resize(QSize(UKWS_ICON_DEFAULT_SIZE, UKWS_ICON_DEFAULT_SIZE));
    closeLabel->resize(QSize(UKWS_ICON_DEFAULT_SIZE, UKWS_ICON_DEFAULT_SIZE));
    titleLabel->setFixedHeight(UKWS_TITLE_DEFAULT_HEIGHT);
    titleLabel->setMinimumWidth(1);
    thumbnailLabel->setMinimumSize(QSize(1, 1));
    closeLabel->setFixedSize(QSize(UKWS_ICON_DEFAULT_SIZE, UKWS_ICON_DEFAULT_SIZE));

    titleLabel->setContentsMargins(0, 0, UKWS_WINDOWBOX_PADDING, 0);

    /*
     * 设置控件布局（简单布局，直接手写）
     *
     *                 +------+-------+-------+   -╮
     * topBarLayout -> | icon | title | close |    |
     *                 +------+-------+-------+    |
     *                 |                      |    | -> mainLayout
     *                 |  thumbnail           |    |
     *                 |                      |    |
     *                 +----------------------+   -╯
     *
     */

//    mainLayout->setSpacing(UKWS_WIDGET_SPACING);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
//    topBarLayout->setSpacing(UKWS_WIDGET_SPACING);
    topBarLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    topBarLayout->addWidget(iconLabel);
    topBarLayout->addWidget(titleLabel);
    topBarLayout->addWidget(closeLabel);

    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(thumbnailLabel);
    this->setLayout(mainLayout);

    // 为缩略图控件注册监视对象
//    iconLabel->installEventFilter(this);
//    titleLabel->installEventFilter(this);
    closeLabel->installEventFilter(this);
    thumbnailLabel->installEventFilter(this);
//    this->installEventFilter(this);

    // 设置缩略图控件的objectName
    titleLabel->setObjectName(UKWS_OBJ_WINBOX_WIN_NAME);
    thumbnailLabel->setObjectName(UKWS_OBJ_WINBOX_THUMBNAIL);
    closeLabel->setObjectName(UKWS_OBJ_WINBOX_CLOSEBTN);
    this->setObjectName(UKWS_OBJ_WINBOX);

    // 对齐设置暂时不生效，使用下面的方式修正
//    mainLayout->setContentsMargins(8, 0, 8, 8);
    mainLayout->setMargin(8);
    setThumbnailNormal();

//    connect(this, &UkwsWindowBox::clicked, this, &UkwsWindowBox::activateWnckWindow);

    scaleTimes = 0;
    scaleTimer.setSingleShot(true);
    scaleTimer.setTimerType(Qt::CoarseTimer);
    scaleTimer.setInterval(UKWS_DRAG_SCALE_INTERVAL_MS);
    connect(&scaleTimer, &QTimer::timeout, this, &UkwsWindowBox::scaleDragPixmap);

    this->setAttribute(Qt::WA_TranslucentBackground, true);
    titleLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    iconLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    thumbnailLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    closeLabel->setAttribute(Qt::WA_TranslucentBackground, true);

    closeLabel->setText("X");
    this->setWindowBoxUnselected();
}

void UkwsWindowBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

    if (isSelected) {
        QSize size = this->size();

        // 获取圆角边框区域
        QRectF rect;
        QPainterPath path;

        rect = QRectF(UKWS_WINDOWBOX_BORDER, UKWS_WINDOWBOX_BORDER,
                      size.width() - UKWS_WINDOWBOX_BORDER * 2,
                      size.height() - UKWS_WINDOWBOX_BORDER * 2);
        path.addRoundedRect(rect, UKWS_THUMBNAIL_RADIUS, UKWS_THUMBNAIL_RADIUS);
        rect = QRectF(0, 0, size.width(), size.height());
        path.addRoundedRect(rect, UKWS_THUMBNAIL_RADIUS + UKWS_WINDOWBOX_BORDER,
                            UKWS_THUMBNAIL_RADIUS + UKWS_WINDOWBOX_BORDER);

        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setBrush(QBrush(QColor(255, 255, 255, 128)));
        painter.setPen(QPen(QColor(255, 255, 255, 0)));
        painter.drawPath(path);
    }

    QWidget::paintEvent(event);
}

void UkwsWindowBox::setSubWidgetSizeByThnSize(int w, int h)
{
    // icon、closeBtn，宽高32，上下间距0，上下边框0，右边距5；
    // 缩略图控件，外边距8，外边框4，边框紧贴图片，宽占用：(8 + 4) * 2 = 24，高占用：8 + 4 = 12；
    // 高度调整值：8 + 32 + 16 = 56；
    // 宽度调整值：(32 + 5) * 2 = 74
    // 标题宽度调整值：
    int fixW, fixH;
    fixW = (UKWS_ICON_DEFAULT_WIDTH + UKWS_WINDOWBOX_PADDING) * 2;
    titleLabel->setFixedSize(w - fixW, UKWS_ICON_DEFAULT_HEIGHT);
    this->updateTitleBySize();

    // 缩略图控件，外边框4，边框紧贴图片，宽、高占用：4 * 2 = 8
    fixW = UKWS_WINDOWBOX_BORDER * 2;
    fixH = UKWS_WINDOWBOX_BORDER * 2;
    thumbnailLabel->setFixedSize(w + fixW, h + fixH);
}

void UkwsWindowBox::setWinboxSizeByHeight(int height)
{
    // winbox边框4，内边距4，宽、高占用：（4 + 4） * 2 = 16；
    // icon，高32，上下间距0，上下边框0，宽、高占用：32；
    // 缩略图控件，外边距8，外边框4，边框紧贴图片，宽占用：(8 + 4) * 2 = 24，高占用：8 + 4 = 12；
    // 高度调整值：16 + 32 + 12 = 60；
    // 宽度调整值：24
    int fixW, fixH, thnHeight;
    fixW = (UKWS_THUMBNAIL_MARGIN + UKWS_WINDOWBOX_BORDER) * 2;
    fixH = (UKWS_WINDOWBOX_BORDER + UKWS_WINDOWBOX_PADDING) * 2
            + UKWS_ICON_DEFAULT_HEIGHT
            + (UKWS_THUMBNAIL_MARGIN + UKWS_WINDOWBOX_BORDER);
    thnHeight = height - fixH;

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

    if (h < thnHeight) {
        h = thnHeight;
    } else {
        h = int(h * scale);
    }

    // 限制最大宽度，调节缩放因子
    if (w * scale > maxWidth)
        scale = (float)maxWidth / w;

    if (w * scale < 184) {
        scale = (float)184 / w;
    }

    // 计算缩略图大小
    w = int(w * scale);


    this->setSubWidgetSizeByThnSize(w, h);
    this->setFixedSize(w + fixW, height);
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

#ifdef NO_ADD_FRAME
    winLeftOffset = 0;
    winRightOffset = 0;
    winTopOffset = 0;
    winBottomOffset = 0;
    frameXid = wnck_window_get_xid(wnckWin);
    hasFrame = false;
#else
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
#endif
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

void UkwsWindowBox::updateThumbnail()
{
    QSize labelSize = thumbnailLabel->size();

    thnUnselectedPixmap = QPixmap(labelSize.width(), labelSize.height());
    thnUnselectedPixmap.fill(Qt::transparent);
//    thnSelectedPixmap = QPixmap(labelSize.width(), labelSize.height());
//    thnSelectedPixmap.fill(Qt::transparent);
    if(thumbnailLabel->originalQPixmap.isNull() == true)
        thumbnailLabel->setPixmap(thnUnselectedPixmap);
    else
    {
        // 自绘边框宽度为4px，纯缩略图大小为长宽需要减8
        QPixmap origThn, radiusThn;


        int fixW = (UKWS_THUMBNAIL_MARGIN + UKWS_WINDOWBOX_BORDER) * 2;
        if (labelSize.width()-fixW >184) {
            origThn = thumbnailLabel->originalQPixmap.scaled(labelSize.width() - UKWS_WINDOWBOX_BORDER * 2,
                                                             labelSize.height() - UKWS_WINDOWBOX_BORDER * 2,
                                                             Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation);
        } else {
            origThn = thumbnailLabel->originalQPixmap.scaled(thumbnailLabel->originalQPixmap.size().width(),
                                                             labelSize.height() - UKWS_WINDOWBOX_BORDER * 2,
                                                             Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation);
        }

        QPainter painter;
        radiusThn = makeRadiusPixmap(origThn, UKWS_THUMBNAIL_RADIUS);
        painter.begin(&thnUnselectedPixmap);
        painter.drawPixmap((thnUnselectedPixmap.width()-radiusThn.width())/2,
                           (thnUnselectedPixmap.height()-radiusThn.height())/2,
                           radiusThn.width(), radiusThn.height(), radiusThn);
        painter.end();
        thnSelectedPixmap = thnUnselectedPixmap.copy();

        // 获取圆角边框区域
        QRectF rect;
        QPainterPath path;

        rect = QRectF(UKWS_WINDOWBOX_BORDER, UKWS_WINDOWBOX_BORDER,
                      labelSize.width() - UKWS_WINDOWBOX_BORDER * 2,
                      labelSize.height() - UKWS_WINDOWBOX_BORDER * 2);
        path.addRoundedRect(rect, UKWS_THUMBNAIL_RADIUS, UKWS_THUMBNAIL_RADIUS);
        rect = QRectF(0, 0, labelSize.width(), labelSize.height());
        path.addRoundedRect(rect, UKWS_THUMBNAIL_RADIUS + UKWS_WINDOWBOX_BORDER,
                            UKWS_THUMBNAIL_RADIUS + UKWS_WINDOWBOX_BORDER);

        painter.begin(&thnSelectedPixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setBrush(QBrush(QColor(255, 255, 255, 128)));
        painter.setPen(QPen(QColor(255, 255, 255, 0)));
        painter.drawPath(path);
        painter.end();
    }
}

void UkwsWindowBox::setThumbnail(QPixmap origPixmap)
{
    thumbnailLabel->originalQPixmap = origPixmap;
    updateThumbnail();
    thumbnailLabel->setPixmap(thnUnselectedPixmap);
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
    updateThumbnail();
    thumbnailLabel->setPixmap(thnUnselectedPixmap);
}

void UkwsWindowBox::setDragIconSize(QSize size)
{
    dragIconSize = size;
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
    if (titleAutoHide) {
        iconLabel->show();
        titleLabel->show();
        closeLabel->show();
    }
    thumbnailLabel->setPixmap(thnSelectedPixmap);
}

void UkwsWindowBox::setThumbnailNormal()
{
    if (titleAutoHide) {
        iconLabel->hide();
        titleLabel->hide();
        closeLabel->hide();
    }
    thumbnailLabel->setPixmap(thnUnselectedPixmap);
}

void UkwsWindowBox::setWindowBoxSelected()
{
    isSelected = true;
    update();
}

void UkwsWindowBox::setWindowBoxUnselected()
{
    isSelected = false;
    update();
}

void UkwsWindowBox::setTitleAutoHide(bool autoHide)
{
    titleAutoHide = autoHide;

    // 设置了自动隐藏则即刻隐藏标题栏
    if (titleAutoHide) {
        iconLabel->hide();
        titleLabel->hide();
        closeLabel->hide();
    }
}

void UkwsWindowBox::moveToWorkspace(int wsIndex)
{
    WnckScreen *screen = wnck_window_get_screen(wnckWin);
    WnckWorkspace *workspace = wnck_screen_get_workspace(screen, wsIndex);
    wnck_window_move_to_workspace(wnckWin, workspace);
}

bool UkwsWindowBox::windowIsAlive()
{
    return WNCK_IS_WINDOW(wnckWin);
}

// 构建圆角图片
QPixmap UkwsWindowBox::makeRadiusPixmap(QPixmap orig, int radius)
{
    // 构造圆角裁剪区域
    QRectF rect = QRectF(0, 0, orig.width(), orig.height());
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);

    // 设置新全透明背景图片
    QPixmap dest(orig.size());
    dest.fill(Qt::transparent);

    // 裁剪圆角区域，将原图贴到裁剪好的区域，构建圆角图片
    QPainter painter(&dest);
    painter.setClipPath(path);
    painter.drawPixmap(orig.rect(), orig);

    return dest;
}

void UkwsWindowBox::scaleDragPixmap()
{
    if (scaleTimes < 0) {
        return;
    }

    if (scaleTimes == 0) {
        return;
    }

    if (drag == nullptr) {
        return;
    }

    scaleTimes--;
    scaleTimer.start();

    // 设置缩放大小
    QSize size = thnUnselectedPixmap.size() - scaleUnitSize * (UKWS_DRAG_SCALE_TIMES - scaleTimes);
    QPixmap pixmap = thnUnselectedPixmap.scaled(size, Qt::KeepAspectRatio,
                                            Qt::FastTransformation);
    drag->setPixmap(pixmap);
    update();
//    drag->setHotSpot(mouseEvent->pos() - thumbnailOffset);


}

bool UkwsWindowBox::eventFilter(QObject *watched, QEvent *event)
{
    static bool thumbnailPressed = false;

    if (watched == thumbnailLabel) {
        if (event->type() == QEvent::Enter) {
            thumbnailPressed = false;
            setThumbnailHover();

            return true;
        }

        if (event->type() == QEvent::Leave) {
            thumbnailPressed = false;
        }

        // 左键按下则标记按下状态
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton)
                thumbnailPressed = true;

            return true;
        }

        // 在同一个WindowBox中按下鼠标并抬起才算选定
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (thumbnailPressed && (mouseEvent->button() == Qt::LeftButton)) {
                thumbnailPressed = false;
                emit clicked(this);

                return true;
            }
        }

        if (event->type() == QEvent::MouseMove) {
            if (!thumbnailPressed)
                return false;

            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            // 禁用拖拽，直接返回
            if (!dragable)
                return true;

            // 缩略图置灰
            isDragged = true;
            QPixmap tempPixmap = thnUnselectedPixmap;
            QPainter painter;
            painter.begin(&tempPixmap);
            painter.fillRect(thnUnselectedPixmap.rect(), QColor(0, 0, 0, 159));
            painter.end();
            thumbnailLabel->setPixmap(tempPixmap);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

            // 拖拽处理
            drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            QByteArray byteData;
            QDataStream dataStream(&byteData, QIODevice::WriteOnly);

            dataStream << parentIndex << index << title;
            mimeData->setData("application/x-dnditemdata", byteData);
            drag->setMimeData(mimeData);
//            drag->setPixmap(thnUnselectedPixmap);
            drag->setPixmap(thnUnselectedPixmap.scaled(dragIconSize, Qt::KeepAspectRatio,
                                                          Qt::FastTransformation));

            // 事件发生时的坐标，减去thn边框的宽度，获得实际图片上的坐标
            QPoint hotSpot = (mouseEvent->pos() - thumbnailOffset) *
                    (float)drag->pixmap().size().width() / (float)thnUnselectedPixmap.width();
            drag->setHotSpot(hotSpot);

            // 设置缩放大小
            scaleUnitSize = (thnUnselectedPixmap.size() - drag->pixmap().size()) / UKWS_DRAG_SCALE_TIMES;

            // 设置拖拽图片缩放动画的定时器
            scaleTimer.start();
            scaleTimes = UKWS_DRAG_SCALE_TIMES;

            if (drag->exec()) {
                thumbnailLabel->setPixmap(thnUnselectedPixmap);
            } else {
                thumbnailLabel->setPixmap(thnUnselectedPixmap);
            }

            drag->deleteLater();
            drag = nullptr;

            isDragged = false;
            setThumbnailNormal();
            return true;
        }
    }

    if (watched == closeLabel) {
        if (event->type() == QEvent::Enter) {
            closeLabel->setStyleSheet("background-color: red;");

            return true;
        }

        if (event->type() == QEvent::Leave) {
            closeLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            closeLabel->setStyleSheet("background-color: blue;");

            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            closeLabel->setStyleSheet("background-color: red;");
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            int limitX = closeLabel->geometry().width();
            int limitY = closeLabel->geometry().height();
            int mouseX = mouseEvent->pos().x();
            int mouseY = mouseEvent->pos().y();

            if ((0 < mouseX) && (mouseX < limitX) &&
                    (0 < mouseY) && (mouseY < limitY)) {
                emit closeBtnClicked(this);
            }

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

void UkwsWindowBox::closeWnckWindow()
{
    unsigned long timestamp = QX11Info::getTimestamp();
    unsigned long xid = wnck_window_get_xid(wnckWin);
    WnckWindow *p = wnck_window_get(xid);
    unsigned long actions = wnck_window_get_actions(wnckWin);

    wnck_window_close(wnckWin, timestamp);
    actions = wnck_window_get_actions(wnckWin);
}

void UkwsWindowBox::leaveEvent(QEvent *)
{
    if (!isDragged)
        setThumbnailNormal();
}
