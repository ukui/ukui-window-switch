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

#ifndef UKWS_WINDOW_MANAGER_H
#define UKWS_WINDOW_MANAGER_H

#include "ukws_common.h"
#include "ukws_indicator.h"
#include "ukws_workspace_manager.h"
#include "ukws_config.h"
#include "qhotkey.h"

#include <QObject>
#include <QWidget>
#include <QLayout>
#include <QList>
#include <QDBusConnection>
#include <QString>
#include <QTimer>
#include <QThread>

#define UKWS_WM_KEYMAP_OFFSET_ALT_LEFT      8
#define UKWS_WM_KEYMAP_OFFSET_ALT_RIGHT     13
#define UKWS_WM_KEYMAP_MASK_ALT_LEFT        0x01
#define UKWS_WM_KEYMAP_MASK_ALT_RIGHT       0x10

#define UKWS_WM_KEY_CHECK_INTERVAL_TIME_MS          10
#define UKWS_WM_SHOW_STATUS_CHECK_INTERVAL_TIME_MS  5

class UkwsAltChecker;
class UkwsManager : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.WindowSwitch")
public:
    explicit UkwsManager(QWidget *parent = nullptr);

    void checkShortcutStatus();
    void checkAltStatus();
    void setGrabKeyboard(bool needGrab);
    void setConfig(UkwsConfig *config);
    void setTheme(QString themeString);

    bool waitingShowStatusStable(UkwsWidgetShowStatus &status, int timeoutMS);

    UkwsConfig *config;
    QTimer altCheckTimer;
    QTimer shortcutCheckTimer;
//    UkwsAltChecker *altChecker;

    UkwsIndicator *ind;
    UkwsWorkspaceManager *ws;

signals:

public slots:
    bool handleWorkspace();
    bool reloadConfig();

private slots:
    void showNextWinbox();
    void showPrevWinbox();
    bool showIndicator();
    void hideIndicator();
    void hideIndicatorAndActivate(bool needActivate);
    bool showWorkspace();
    void hideWorkspace();

private:
    QHotkey *nextShortcut;
    QHotkey *prevShortcut;
    QHotkey *workspaceShortcut1;
    QHotkey *workspaceShortcut2;
};

class UkwsAltChecker : public QThread
{
    Q_OBJECT

    enum UkwsAltCheckerStatus {
        Running,
        Stopping,
        Stopped,
    };

public:
    explicit UkwsAltChecker(QObject *parent = nullptr);
    void run() override;
    bool stop(int timeoutMS);

signals:
    void altReleased();

private:
    UkwsAltCheckerStatus status;
};

#endif // UKWS_WINDOW_MANAGER_H
