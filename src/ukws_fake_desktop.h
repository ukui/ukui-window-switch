#ifndef UKWS_FAKE_DESKTOP_H
#define UKWS_FAKE_DESKTOP_H

#include "ukws_common.h"
#include "ukws_window_info.h"
#include "ukws_wnck_operator.h"
#include "ukws_config.h"
#include "ukws_worker.h"

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QList>
#include <QString>
#include <QInputEvent>

class UkwsFakeDesktop : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsFakeDesktop(QWidget *parent = nullptr);

    void setBackground(QPixmap background);
    void reSetWindowThumbnailByWnck();
    void reloadWindowList();
    void reShow();

    void cleanWininfo();

    bool updateWindowViewPixmap(bool newRequest=false);

    int index;
    UkwsConfig *config;
    UkwsWnckOperator *wmOperator;

protected:
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void windowViewPixmapChange(int index);

public slots:
    void doWorkerDone();

private:
    QLabel *viewLabel;
    QVBoxLayout *mainLayout;
    QList<UkwsWorker *> workerList;
    QList<UkwsWindowInfo *> wininfoList;

    bool dragWindow;
    int cpus;
    int updateDesktopViewRequestId;
    int updateDesktopViewHandledId;
    QPoint dragPos;
    QPixmap background;
    QPixmap windowViewPixmap;
    QPixmap desktopViewPixmap;
};

#endif // UKWS_FAKE_DESKTOP_H
