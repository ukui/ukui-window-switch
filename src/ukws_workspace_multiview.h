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
