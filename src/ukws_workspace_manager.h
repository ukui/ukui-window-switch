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

#ifndef UKWS_WORKSPACE_MANAGER_H
#define UKWS_WORKSPACE_MANAGER_H

#include "ukws_indicator.h"
#include "ukws_flowlayout.h"
#include "ukws_wnck_operator.h"
#include "ukws_workspace_box.h"
#include "ukws_new_workspace_button.h"
#include "ukws_config.h"

#include <QWidget>
//#include <QMainWindow>
#include <QLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include <QString>
#include <QStackedWidget>
#include <QPixmap>
#include <QGSettings>

#define UKWS_WORKSPACE_MAX_SCALE 3
#define UKWS_WORKSPACE_DEFAULT_BACKGROUND   "/usr/share/ukui-window-switch/data/default-background.png"
#define UKWS_WORKSPACE_MAX  4

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
    QString getBackgroundColorGSettings(QString schemaDir, QString schemaUrl, QString keyName);
    void getBackground();
    void setBackgroundImage(int width = 0, int height = 0);
    void cleanAllWorkspace();

    void setConfig(UkwsConfig *config);

    UkwsConfig *config;
    UkwsWidgetShowStatus showStatus;

signals:
    void isHidden();

public slots:
//    void onCloseButtonRealsed();
    void setShowingIndicator(int index);
    void changeWorkspace(int index);
    void selectWinbox(bool needActivate);
    void moveWindowWorkspace(int wbIndex, int srcWsIndex, int dstWsIndex);
    void doIndicatorWindowViewChange(int indIndex);

private:
    UkwsWnckOperator *wmOperator;
    QList<UkwsIndicator *> indList;
    QList<UkwsWorkspaceBox *> spaceBoxList;

    QStackedWidget *indStack;
    UkwsNewWorkspaceButton *newWorkspaceBtn;

    QHBoxLayout *mainLayout;
    QVBoxLayout *wsboxLayout;
    QSpacerItem *topSpacer;

    QPixmap background;
};

#endif // UKWS_WORKSPACE_MANAGER_H
