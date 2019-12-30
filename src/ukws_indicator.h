#ifndef UKWS_INDICATOR_H
#define UKWS_INDICATOR_H

#include "ukws_common.h"
#include "ukws_window_box.h"
#include "ukws_flowlayout.h"
#include "ukws_wnck_operator.h"
#include "ukws_worker.h"

#include <QWidget>
#include <QLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include <QString>
#include <QInputEvent>

#define UKWS_WINBOX_SWITCH_WIDTH   (168 + 5 + 5)
#define UKWS_WINBOX_SWITCH_HEIGHT   (96 + 0 + 32 + 5 + 5)
#define UKWS_WINBOX_MAX_SCALE   4

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

    void addWinBox(UkwsWindowBox *winbox);
    void cleanWinBox(int index);
    void cleanAllWinBox();
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

    UkwsWnckOperator *wmOperator;

    UkwsIndicatorShowMode showMode;
    UkwsWidgetShowStatus showStatus;
    int selIndex;

protected:
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void isSelected(bool needActivate);

public slots:
    void clickWinbox(UkwsWindowBox *wb);
    void reShow(UkwsIndicatorShowMode mode=ShowModeUnknown, int minScale=UKWS_WINBOX_MAX_SCALE);

private:
    void moveWindow(void);
    void layoutTest(int maxWidth, int *maxHight, int *maxBlankWidth);

    QList<UkwsWorker *> workerList;
    QList<UkwsWindowBox *> winboxList;
    UkwsFlowLayout *winboxFlowLayout;
    QWidget *flowArea;
    QScrollArea *flowScrollArea;

    QVBoxLayout *mainLayout;

    float m_minRatio;

    bool dragWindow;
    bool hasStopSignal;
    int cpus;
    QPoint dragPos;
};

#endif // UKWS_INDICATOR_H
