#ifndef UKWS_WORKSPACE_MANAGER_H
#define UKWS_WORKSPACE_MANAGER_H

#include "ukws_indicator.h"
#include "ukws_flowlayout.h"
#include "ukws_wnck_operator.h"
#include "ukws_workspace_box.h"

#include <QWidget>
//#include <QMainWindow>
#include <QLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include <QString>
#include <QStackedWidget>
#include <QPixmap>

#define UKWS_WORKSPACE_MAX_SCALE 3
#define UKWS_WORKSPACE_DEFAULT_BACKGROUND   "/usr/share/ukui-window-switch/data/default-background.png"

class UkwsWorkspaceManager: public QWidget
{
    Q_OBJECT
public:
    explicit UkwsWorkspaceManager(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *object, QEvent *event);

    void reloadWorkspace(int minScale);
    void reShow(int minScale=UKWS_WORKSPACE_MAX_SCALE);
    void reHide();
    QString getBackgroundFileByGSettings(QString schemaDir, QString schemaUrl, QString keyName);
    void getBackground();
    void setBackgroundImage();
    void cleanAllWorkspace();

    UkwsWidgetShowStatus showStatus;

signals:
    void isHidden();

public slots:
//    void onCloseButtonRealsed();
    void setShowingIndicator(int index);
    void changeWorkspace(int index);
    void selectWinbox(bool needActivate);
    void moveWindowWorkspace(int wbIndex, int srcWsIndex, int dstWsIndex);

private:
    UkwsWnckOperator *wmOperator;
    QList<UkwsIndicator *> indList;
    QList<UkwsWorkspaceBox *> spaceBoxList;

    QStackedWidget *indStack;

    QHBoxLayout *mainLayout;
    QVBoxLayout *wsboxLayout;

    QPixmap background;
};

#endif // UKWS_WORKSPACE_MANAGER_H
