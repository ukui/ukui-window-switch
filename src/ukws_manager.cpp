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

#include "ukws_manager.h"

#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QDBusError>
#include <QX11Info>
#include <QTime>

extern "C" {
#include <X11/Xlib.h>
}

UkwsManager::UkwsManager(QWidget *parent) : QWidget(parent)
{
    nextShortcut = new QHotkey(QKeySequence(Qt::AltModifier |
                                            Qt::Key_Tab),
                               true, this);
    prevShortcut = new QHotkey(QKeySequence(Qt::AltModifier |
                                            Qt::ShiftModifier |
                                            Qt::Key_Tab),
                               true, this);
    workspaceShortcut = new QHotkey(QKeySequence(Qt::MetaModifier |
                                                 Qt::Key_Tab),
                                    true, this);
//    workspaceShortcut = new QHotkey(QKeySequence(Qt::AltModifier |
//                                                 Qt::Key_A),
//                                    true, this);
    connect(nextShortcut, &QHotkey::activated, this, &UkwsManager::showNextWinbox);
    connect(prevShortcut, &QHotkey::activated, this, &UkwsManager::showPrevWinbox);
    connect(workspaceShortcut, &QHotkey::activated, this, &UkwsManager::handleWorkspace);

    // 每10s检测一次热键注册情况
    shortcutCheckTimer.setTimerType(Qt::CoarseTimer);
    shortcutCheckTimer.setInterval(UKWS_WM_KEY_CHECK_INTERVAL_TIME_MS * 1000);
    connect(&(this->shortcutCheckTimer), &QTimer::timeout, this, &UkwsManager::checkShortcutStatus);
    shortcutCheckTimer.start();

    // 每10ms检测一次Alt键状态
    altCheckTimer.setTimerType(Qt::CoarseTimer);
    altCheckTimer.setInterval(UKWS_WM_KEY_CHECK_INTERVAL_TIME_MS);
    connect(&(this->altCheckTimer), &QTimer::timeout, this, &UkwsManager::checkAltStatus);

    config = nullptr;
    ind = new UkwsIndicator;
    ws = new UkwsWorkspaceManager;
//    altChecker = new UkwsAltChecker;

    ind->showMode = UkwsIndicator::ShowModeSwitch;
    // 窗口指示器显示所有工作区中的窗口，但是不显示非当前屏幕的窗口
    ind->wmOperator->needCheckWorkspace = false;
    ind->wmOperator->needCheckScreen = true;
    ind->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    ws->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    connect(ind, &UkwsIndicator::isSelected, this, &UkwsManager::hideIndicatorAndActivate);
    connect(ws, &UkwsWorkspaceManager::isHidden, this, &UkwsManager::hideWorkspace);
//    connect(altChecker, &UkwsAltChecker::altReleased, this, &UkwsManager::hideIndicator);

    // 连接session总线
    QDBusConnection connection = QDBusConnection::sessionBus();

    // 在session总线上为UKWS注册服务
    if(!connection.registerService("org.ukui.WindowSwitch")) {
        qCritical() << "Register DBus Service Error:" << connection.lastError().message();
    }
    // 注册org.ukui.WindowSwitch服务的object，把UkwsManager类的所有公共槽函数导出为object的method
    connection.registerObject("/org/ukui/WindowSwitch", this, QDBusConnection::ExportAllSlots);
}

void UkwsManager::setTheme(QString themeString)
{
    ind->setStyleSheet(themeString);
    ws->setStyleSheet(themeString);
}

bool UkwsManager::waitingShowStatusStable(UkwsWidgetShowStatus &status, int timeoutMS)
{
    QTime curTime = QTime::currentTime();
    curTime.start();

    while (curTime.elapsed() < timeoutMS) {
        if ((status == UkwsWidgetShowStatus::Hidden) ||
                (status == UkwsWidgetShowStatus::Shown) ||
                (status == UkwsWidgetShowStatus::Interrupted))
            return true;

        usleep(UKWS_WM_SHOW_STATUS_CHECK_INTERVAL_TIME_MS * 1000);
        // 处理其他事件，不阻塞主进程
//        QCoreApplication::processEvents();
    }

    qWarning() << "UkwsManager::waitingShowStatusStable timeout:" << timeoutMS << "ms";
    return false;
}

bool UkwsManager::showIndicator()
{
    if (ind->showStatus == UkwsWidgetShowStatus::Shown)
        return true;

    if (!altCheckTimer.isActive())
        altCheckTimer.start();

    // 等待指示器构造或者析构完毕，等待1s钟，超时则不继续之后流程
    if (!waitingShowStatusStable(ind->showStatus, 1000))
        return false;

    if (ind->showStatus == UkwsWidgetShowStatus::Hidden) {
        setGrabKeyboard(true);
        ind->reShow();

        return true;
    }

    return false;
}

void UkwsManager::showNextWinbox()
{
    if (ind->showStatus == UkwsWidgetShowStatus::Hidden) {
        ind->selIndex = 0;
    }

    showIndicator();
    ind->selectWindow(ind->selIndex + 1);
}

void UkwsManager::showPrevWinbox()
{
    if (ind->showStatus == UkwsWidgetShowStatus::Hidden) {
        ind->selIndex = 0;
    }

    showIndicator();
    ind->selectWindow(ind->selIndex - 1);
}

void UkwsManager::hideIndicator()
{
    hideIndicatorAndActivate(true);
}

void UkwsManager::hideIndicatorAndActivate(bool needActivate)
{
    if (ind->showStatus == UkwsWidgetShowStatus::Constructing)
        ind->stopConstructing(100000);

    if (ind->showStatus == UkwsWidgetShowStatus::Destructing)
        return;

    if ((ind->showStatus == UkwsWidgetShowStatus::Shown) ||
            (ind->showStatus == UkwsWidgetShowStatus::Interrupted)) {
        altCheckTimer.stop();
        ind->reHide(needActivate);
        setGrabKeyboard(false);
    }
}

void UkwsManager::checkShortcutStatus()
{
    if (!nextShortcut->isRegistered()) {
        nextShortcut->setRegistered(true);
    }

    if (!prevShortcut->isRegistered()) {
        prevShortcut->setRegistered(true);
    }

    if (!workspaceShortcut->isRegistered()) {
        workspaceShortcut->setRegistered(true);
    }
}

void UkwsManager::checkAltStatus()
{
    bool leftAltReleased = false;
    bool rightAltReleased = false;
    char keymap[32] = {0};
    static Display *display = NULL;

    display = QX11Info::display();

    XQueryKeymap(display, keymap);

    if ((keymap[UKWS_WM_KEYMAP_OFFSET_ALT_LEFT] & UKWS_WM_KEYMAP_MASK_ALT_LEFT) == 0)
        leftAltReleased = true;

    if ((keymap[UKWS_WM_KEYMAP_OFFSET_ALT_RIGHT] & UKWS_WM_KEYMAP_MASK_ALT_RIGHT) == 0)
        rightAltReleased = true;

    if (leftAltReleased && rightAltReleased) {
        altCheckTimer.stop();
        hideIndicator();
    }
}

void UkwsManager::setGrabKeyboard(bool needGrab)
{
    XSync(QX11Info::display(), False);
    if (needGrab) {
        XGrabKeyboard(QX11Info::display(), QX11Info::appRootWindow(),
                      True, GrabModeAsync, GrabModeAsync, CurrentTime);
    } else {
        XUngrabKeyboard(QX11Info::display(), CurrentTime);
    }
    XFlush(QX11Info::display());
}

bool UkwsManager::handleWorkspace()
{
    // 等待指示器构造或者析构完毕，等待1s钟，超时则不继续之后流程
    if (!waitingShowStatusStable(ws->showStatus, 1000))
        return false;

    if (ws->showStatus == UkwsWidgetShowStatus::Hidden) {
        showWorkspace();
        return true;
    }

    if (ws->showStatus == UkwsWidgetShowStatus::Shown) {
        hideWorkspace();
        return true;
    }

    return false;
}

bool UkwsManager::showWorkspace()
{
    setGrabKeyboard(true);
    ws->reShow();

    // 处理所有show事件，完成初始布局
    QCoreApplication::processEvents();

    // 显示当前的工作区
    QDesktopWidget *desktop = QApplication::desktop();
    int screenNum = desktop->screenNumber(this);
    WnckScreen *screen = wnck_screen_get(screenNum);
    WnckWorkspace *workspace = wnck_screen_get_active_workspace(screen);
    int workspaceNum = wnck_workspace_get_number(workspace);
    ws->setShowingIndicator(workspaceNum);

    return true;
}

void UkwsManager::hideWorkspace()
{
    // 当workspace manager显示的时候才需要隐藏，来自isHidden信号则不需要再次隐藏
    if (ws->showStatus == UkwsWidgetShowStatus::Shown)
        ws->reHide();

    setGrabKeyboard(false);
}

UkwsAltChecker::UkwsAltChecker(QObject *parent)
    :QThread(parent)
{
    status = UkwsAltChecker::Stopped;
}

void UkwsAltChecker::run()
{
    char keymap[32] = {0};
    static Display *display = NULL;

    display = QX11Info::display();
    bool leftAltReleased = false;
    bool rightAltReleased = false;
    status = UkwsAltChecker::Running;

    while (status == UkwsAltChecker::Running) {
        leftAltReleased = false;
        rightAltReleased = false;

        XQueryKeymap(display, keymap);

        if ((keymap[UKWS_WM_KEYMAP_OFFSET_ALT_LEFT] & UKWS_WM_KEYMAP_MASK_ALT_LEFT) == 0)
            leftAltReleased = true;

        if ((keymap[UKWS_WM_KEYMAP_OFFSET_ALT_RIGHT] & UKWS_WM_KEYMAP_MASK_ALT_RIGHT) == 0)
            rightAltReleased = true;

        if (leftAltReleased && rightAltReleased) {
            status = UkwsAltChecker::Stopped;
            emit altReleased();
            return;
        }

        usleep(UKWS_WM_KEY_CHECK_INTERVAL_TIME_MS * 1000);
    }

    status = UkwsAltChecker::Stopped;
}

bool UkwsAltChecker::stop(int timeoutMS)
{
    if (status == UkwsAltChecker::Stopped)
        return true;

    if (status == UkwsAltChecker::Running)
        status = UkwsAltChecker::Stopping;

    QTime curTime = QTime::currentTime();
    curTime.start();

    while (curTime.elapsed() < timeoutMS) {
        if (status == UkwsAltChecker::Stopped)
            return true;

        usleep(UKWS_WM_SHOW_STATUS_CHECK_INTERVAL_TIME_MS * 1000);
    }

    return false;
}

