#ifndef UKWS_WORKSPACE_MULTIVIEW_H
#define UKWS_WORKSPACE_MULTIVIEW_H

#include "ukws_fake_desktop.h"
#include "ukws_wnck_operator.h"
#include "ukws_config.h"

#include <QWidget>
#include <QLayout>
#include <QList>
#include <QString>
#include <QPixmap>

#define UKWS_WORKSPACE_MULTIVIEW_LINE_MAX 2
#define UKWS_WORKSPACE_DEFAULT_BACKGROUND   "/usr/share/ukui-window-switch/data/default-background.png"

class UkwsWorkspaceMultiview : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsWorkspaceMultiview(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *object, QEvent *event);

    void reloadWorkspace(int lineMax);
    void reShow(int lineMax=UKWS_WORKSPACE_MULTIVIEW_LINE_MAX);
    void reHide();
    QString getBackgroundFileByGSettings(QString schemaDir, QString schemaUrl, QString keyName);
    void getBackground();
    void setBackgroundImage();
    void cleanAllWorkspace();

    void setConfig(UkwsConfig *config);

    UkwsConfig *config;
    UkwsWidgetShowStatus showStatus;

signals:

public slots:

private:
    UkwsWnckOperator *wmOperator;
    QList<UkwsFakeDesktop *> fakeDesktopList;

    QGridLayout *mainLayout;

    QPixmap background;
};

#endif // UKWS_WORKSPACE_MULTIVIEW_H
