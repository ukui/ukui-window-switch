#include "ukws_manager.h"

#include <QCoreApplication>
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
    connect(this, &UkwsManager::indShow, ind, &UkwsIndicator::reShow);
    connect(ws, &UkwsWorkspaceManager::isHidden, this, &UkwsManager::hideWorkspace);
//    connect(altChecker, &UkwsAltChecker::altReleased, this, &UkwsManager::hideIndicator);

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

//    if (!altChecker->isRunning()) {
//        qDebug() << "alt checker running";
//        altChecker->start();
//    }
    if (!altCheckTimer.isActive())
        altCheckTimer.start();

    // 等待指示器构造或者析构完毕，等待1s钟，超时则不继续之后流程
    if (!waitingShowStatusStable(ind->showStatus, 1000))
        return false;

    if (ind->showStatus == UkwsWidgetShowStatus::Hidden) {
        setGrabKeyboard(true);
        qDebug() << "Will show" << ind->showStatus;
        ind->cleanStopSignal();
        ind->reShow();
        qDebug() << "reShow done";
//        emit indShow();

        return true;
    }

    return false;
}

void UkwsManager::showNextWinbox()
{
    qDebug() << "showIndicator +";
    if (ind->showStatus == UkwsWidgetShowStatus::Hidden) {
        ind->selIndex = 0;
    }

    showIndicator();
    ind->selectWindow(ind->selIndex + 1);
}

void UkwsManager::showPrevWinbox()
{
    qDebug() << "showIndicator -";
    if (ind->showStatus == UkwsWidgetShowStatus::Hidden) {
        ind->selIndex = 0;
    }

    showIndicator();
    ind->selectWindow(ind->selIndex - 1);
}

void UkwsManager::hideIndicator()
{
    qDebug() << "hideIndicator" << ind->showStatus;
    hideIndicatorAndActivate(true);
}

void UkwsManager::hideIndicatorAndActivate(bool needActivate)
{
    if (ind->showStatus == UkwsWidgetShowStatus::Constructing)
        ind->stopConstructing(100000);

    qDebug() << "hideIndicatorAndActivate" << ind->showStatus;
    if (ind->showStatus == UkwsWidgetShowStatus::Destructing)
        return;

    if ((ind->showStatus == UkwsWidgetShowStatus::Shown) ||
            (ind->showStatus == UkwsWidgetShowStatus::Interrupted)) {
        altCheckTimer.stop();
//        if (!altChecker->stop(1000))
//            altChecker->quit();
        qDebug() << "will reHide";
        ind->reHide(needActivate);
        qDebug() << "reHide done";
        setGrabKeyboard(false);
    }
}

void UkwsManager::checkShortcutStatus()
{
    if (!nextShortcut->isRegistered()) {
        qDebug() << "Re-register next shortcut";
        nextShortcut->setRegistered(true);
    }

    if (!prevShortcut->isRegistered()) {
        qDebug() << "Re-register prev shortcut";
        prevShortcut->setRegistered(true);
    }

    if (!workspaceShortcut->isRegistered()) {
        qDebug() << "Re-register next workspace shortcut";
        workspaceShortcut->setRegistered(true);
    }
}

void UkwsManager::checkAltStatus()
{
//    qDebug() << "checkAltStatus";
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
        qDebug() << "===== release =====" ;
//        if (ind->stopConstructing())
//            qDebug() << "ind->stopConstructing: True";
//        else
//            qDebug() << "ind->stopConstructing: False";
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
    qDebug() << "handleworkspace";
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
    ws->setShowingIndicator(0);

    return true;
}

void UkwsManager::hideWorkspace()
{
    qDebug() << "UkwsManager hideWorkspace start";

    // 当workspace manager显示的时候才需要隐藏，来自isHidden信号则不需要再次隐藏
    if (ws->showStatus == UkwsWidgetShowStatus::Shown)
        ws->reHide();

    setGrabKeyboard(false);
    qDebug() << "UkwsManager hideWorkspace done";
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
            qDebug() << "leftAltReleased && rightAltReleased";
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

    qDebug() << "alt checker stop";

    QTime curTime = QTime::currentTime();
    curTime.start();

    while (curTime.elapsed() < timeoutMS) {
        if (status == UkwsAltChecker::Stopped)
            return true;

        usleep(UKWS_WM_SHOW_STATUS_CHECK_INTERVAL_TIME_MS * 1000);
    }

    return false;
}

