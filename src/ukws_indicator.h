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

#ifndef UKWS_INDICATOR_H
#define UKWS_INDICATOR_H

#include "ukws_common.h"
#include "ukws_window_box.h"
#include "ukws_flowlayout.h"
#include "ukws_wnck_operator.h"
#include "ukws_config.h"
#include "ukws_worker.h"

#include <QWidget>
#include <QLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include <QString>
#include <QInputEvent>
#include <QTimer>

//#define UKWS_WINBOX_SWITCH_WIDTH   (168 + 5 + 5)
//#define UKWS_WINBOX_SWITCH_HEIGHT   (96 + 0 + 32 + 5 + 5)
#define UKWS_WINBOX_MAX_SCALE   4

#define UKWS_WINDOW_LIST_CHECK_INTERVAL_MS  10

typedef struct indicator_size_test_t indicator_size_test;
struct indicator_size_test_t {
    int width;
    int height;
    int maxBlankWidth;
};

class UkwsIndicator : public QWidget
{
    Q_OBJECT
public:
    enum UkwsIndicatorShowMode {
        ShowModeUnknown = 0,
        ShowModeSwitch,
        ShowModeTiling,
    };

    explicit UkwsIndicator(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);

    void addWinbox(UkwsWindowBox *winbox);
    void rmWinbox(UkwsWindowBox *winbox);
    void cleanAllWinbox();
    UkwsWindowBox * getWinbox(int winboxIndex);
    void reloadWindowList(int boxMinHeight=0);
    void reSetWindowThumbnailByWnck();
//    void reShow(UkwsIndicatorShowMode mode=ShowModeUnknown, int minScale=UKWS_WINBOX_MAX_SCALE);
    void reHide(bool needActivate);
    bool stopConstructing(int timeoutMS=100);
    void cleanStopSignal();
    void acitveSelectedWindow();
    void flowReLayout();
    void flowReLayout(int *maxWidth, int *maxHeight);
    QSize getMaxRect(const QRect rect, int spaceX=0, int spaceY=0);

    void selectWindow(int index);
    void selectPrevWindow();
    void selectNextWindow();

    QPixmap getWindowView();
    bool updateWindowViewPixmap(bool newRequest=false);

    void setConfig(UkwsConfig *config);

    void checkWindowList();

    UkwsWnckOperator *wmOperator;
    UkwsConfig *config;

    UkwsIndicatorShowMode showMode;
    UkwsWidgetShowStatus showStatus;
    int index;
    int selIndex;
    QTimer windowListcheckTimer;

protected:
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void isSelected(bool needActivate);
    void windowViewPixmapChange(int index);

public slots:
    void doWorkerDone();
    void clickWinbox(UkwsWindowBox *wb);
    void closeWinbox(UkwsWindowBox *wb);
    void reShow(UkwsIndicatorShowMode mode=ShowModeUnknown, int minScale=UKWS_WINBOX_MAX_SCALE);

private:
    void moveWindow(void);
    void layoutTest(int maxWidth, int *maxHight, int *maxBlankWidth);

    QList<UkwsWorker *> workerList;
    QList<UkwsWindowBox *> winboxList;
    UkwsFlowLayout *winboxFlowLayout;
    QWidget *flowArea;
    QScrollArea *flowScrollArea;
    QScrollBar *flowScrollBar;

    QVBoxLayout *mainLayout;

    float m_minRatio;

    bool dragWindow;
    bool hasStopSignal;
    int cpus;
    int updateDesktopViewRequestId;
    int updateDesktopViewHandledId;
    QPoint dragPos;
    QPixmap windowViewPixmap;
};

#endif // UKWS_INDICATOR_H
