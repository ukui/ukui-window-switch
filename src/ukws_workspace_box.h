#ifndef UKWSWORKSPACEBOX_H
#define UKWSWORKSPACEBOX_H

extern "C" {
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <xcb/xcb.h>
}

#include "ukws_window_extra_label.h"
#include "ukws_wnck_operator.h"

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QPixmap>
#include <QSize>
#include <QEvent>
#include <QPushButton>

class UkwsWorkspaceBox : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsWorkspaceBox(QWidget *parent = nullptr);

    QString getTitle();
    void setTitle(QString title);
    void updateTitleBySize();

    WnckWorkspace *getWnckWorkspace();
    void setWnckWorkspace(WnckWorkspace *workspace);

    void setThumbnail(QPixmap thumbnail);
    void setTitleStyle(QString style);
    void setBoxStyle(QString style);

    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *);

    int index;
    UkwsWnckOperator *wmOperator;

signals:
    void doHover(int index);
    void selectedWorkspace(int index);
    void windowChangeWorkspace(int wbIndex, int srcWsIndex, int dstWsIndex);

public slots:
    void onCloseButtonRealsed();
//    void activateWnckWorkspace();

private:
    QString title;

    UkwsWindowExtraLabel *titleLabel;
    UkwsWindowExtraLabel *thumbnailLabel;
    QPushButton *closeButton;

    QVBoxLayout *mainLayout;
    QHBoxLayout *topBarLayout;

    WnckWorkspace *wnckWorkspace;
};

#endif // UKWSWORKSPACEBOX_H
