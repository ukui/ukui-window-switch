#include "ukws_window_box.h"

#include <QString>
#include <QDate>
#include <QSize>
#include <QEvent>
#include <QResizeEvent>
#include <QX11Info>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>

#include "ukws_common.h"
#include "ukws_helper.h"

static const QSize UKWS_TITLE_SIZE = QSize(UKWS_TITLE_WIDTH, UKWS_TITLE_HEIGHT);
static const QSize UKWS_ICON_SIZE = QSize(UKWS_ICON_DEFAULT_WIDTH, UKWS_ICON_DEFAULT_HEIGHT);
static const QSize UKWS_THUMBNAIL_SIZE = QSize(UKWS_THUMBNAIL_DEFAULT_WIDTH, UKWS_THUMBNAIL_DEFAULT_HEIGHT);

UkwsWindowBox::UkwsWindowBox(QWidget *parent) : QWidget(parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground, true);

    titleLabel = new UkwsWindowExtraLabel();
    iconLabel = new UkwsWindowExtraLabel();
    thumbnailLabel = new UkwsWindowExtraLabel();

    titleSize = UKWS_TITLE_SIZE;
    iconSize = UKWS_ICON_SIZE;
    thumbnailSize = UKWS_THUMBNAIL_SIZE;
    winboxSize = QSize(UKWS_WINDOWBOX_WIDTH, UKWS_WINDOWBOX_HEIGHT);

    iconLabel->setAlignment(Qt::AlignCenter);
    thumbnailLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

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
//    thumbnailLabel->setMaximumSize(QSize(300, 200));
    thumbnailLabel->setMinimumSize(QSize(1, 1));

    iconLabel->setContentsMargins(5, 0, 0, 0);
    titleLabel->setContentsMargins(0, 0, 5, 0);
    thumbnailLabel->setContentsMargins(5, 5, 5, 5);

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
    mainLayout->setAlignment(Qt::AlignTop);
    topBarLayout->setSpacing(UKWS_WIDGET_SPACING);
    topBarLayout->setAlignment(Qt::AlignLeft);

    topBarLayout->addWidget(iconLabel);
    topBarLayout->addWidget(titleLabel);

    mainLayout->addLayout(topBarLayout);
    this->setLayout(mainLayout);
    mainLayout->addWidget(thumbnailLabel);

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
    titleLabel->setFixedSize(w - 5 - 32 - 5, 32);
    this->updateTitleBySize();
    thumbnailLabel->setFixedSize(w - 5 - 5, h - 0 - 32 - 5 - 5 - 5);
}

void UkwsWindowBox::setWinboxSizeByHeight(int height)
{
    int thn_height = height - 0 - 32 - 5 - 5;

    int w, h;
    float scale;
    float max_scale = 20.0 / 9;
    int max_w = int(thn_height * max_scale);
    w = thumbnailLabel->originalQPixmap.size().width();
    h = thumbnailLabel->originalQPixmap.size().height();

    if (h > thn_height)
        scale = (float)thn_height / h;
    else
        scale = 1;

    if (w * scale > max_w)
        scale = (float)max_w / w;

    w = int(w * scale);
    h = int(h * scale);

    this->setSubWidgetSize(w + 5 + 5, h + 0 + 32 + 5 + 5);
    this->setFixedSize(w + 5 + 5, height);
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
                                             titleLabel->width() - 10);
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

void UkwsWindowBox::setOrigThumbnailByWnck()
{
    thumbnailLabel->originalQPixmap = UkwsHelper::getThumbnailByXid(wnck_window_get_xid(wnckWin));
}

void UkwsWindowBox::setThumbnail(QPixmap thumbnail)
{
    thumbnailLabel->originalQPixmap = thumbnail;
    thumbnailLabel->setPixmap(thumbnailLabel->originalQPixmap
                              .scaled(thumbnailSize.width(), thumbnailSize.height(),
                                      Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void UkwsWindowBox::setThumbnailByWnck()
{
    QSize size = thumbnailLabel->size();
    if (size.width() == 0 || size.height() == 0)
        thumbnailLabel->originalQPixmap = UkwsHelper::getThumbnailByXid(wnck_window_get_xid(wnckWin));
    thumbnailLabel->setPixmap(thumbnailLabel->originalQPixmap
                              .scaled(size.width() , size.height(),
                                      Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

        // 在同一个WindowBox中按下鼠标并抬起才算选定
        if (event->type() == QEvent::MouseButtonPress) {
            pressed = true;

            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            if (pressed) {
                pressed = false;
                emit clicked(this);

                return true;
            }
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
