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

#define UKWS_WIDGET_SPACING         3
#define UKWS_ADDITIONAL_SPACING    10
#define UKWS_BORDER_WIDTH           2

#define UKWS_ICON_DEFAULT_WIDTH     32
#define UKWS_ICON_DEFAULT_HEIGHT    32

#define UKWS_THUMBNAIL_DEFAULT_WIDTH     320
#define UKWS_THUMBNAIL_DEFAULT_HEIGHT    180

#define UKWS_TITLE_WIDTH     (UKWS_THUMBNAIL_DEFAULT_WIDTH - UKWS_ICON_DEFAULT_WIDTH - UKWS_WIDGET_SPACING)
#define UKWS_TITLE_HEIGHT    UKWS_ICON_DEFAULT_HEIGHT

#define UKWS_WINDOWBOX_WIDTH     (UKWS_THUMBNAIL_DEFAULT_WIDTH + UKWS_BORDER_WIDTH * 2)
#define UKWS_WINDOWBOX_HEIGHT    (UKWS_ICON_DEFAULT_HEIGHT + UKWS_THUMBNAIL_DEFAULT_HEIGHT + UKWS_WIDGET_SPACING + UKWS_ADDITIONAL_SPACING)

class UkwsWindowBox : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsWindowBox(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);

    void setSubWidgetSize(int w, int h);
    void setWinboxSizeByHeight(int height);

    QString getTitle();
    void setTitle(QString title);
    void updateTitleBySize();

    WnckWindow *getWnckWindow();
    void setWnckWindow(WnckWindow *window);

    void setIcon(QPixmap icon);
    void setIconByWnck();
    void setOrigThumbnailByWnck();
    void setThumbnail(QPixmap thumbnail);
    void setThumbnailByWnck();

    void setThumbnailHover();
    void setThumbnailNormal();
    void setWindowBoxSelected();
    void setWindowBoxUnselected();

    bool eventFilter(QObject *watched, QEvent *event);

    int index;
    int winType;

    QSize iconSize;
    QSize titleSize;
    QSize thumbnailSize;
    QSize thnRealSize;
    QSize winboxSize;
    QRect windowRect;

    Qt::TransformationMode iconTransformationMode;
    Qt::TransformationMode thumbnailTransformationMode;

signals:
    void clicked(UkwsWindowBox *winbox);

public slots:
    void activateWnckWindow();

private:
    QString title;

    UkwsWindowExtraLabel *titleLabel;
    UkwsWindowExtraLabel *iconLabel;
    UkwsWindowExtraLabel *thumbnailLabel;

    QVBoxLayout *mainLayout;
    QHBoxLayout *topBarLayout;

    WnckWindow *wnckWin;
};

#endif // UKWS_WINDOWBOX_H
