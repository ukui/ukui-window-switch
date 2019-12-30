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
public:
    explicit UkwsManager(QWidget *parent = nullptr);

    void checkShortcutStatus();
    void checkAltStatus();
    void setGrabKeyboard(bool needGrab);
    void setTheme(QString themeString);

    bool waitingShowStatusStable(UkwsWidgetShowStatus &status, int timeoutMS);

    UkwsConfig *config;
    QTimer altCheckTimer;
    QTimer shortcutCheckTimer;
//    UkwsAltChecker *altChecker;

    UkwsIndicator *ind;
    UkwsWorkspaceManager *ws;

signals:
    void indShow(UkwsIndicator::UkwsIndicatorShowMode
                 mode=UkwsIndicator::UkwsIndicatorShowMode::ShowModeUnknown,
                 int minScale=UKWS_WINBOX_MAX_SCALE);

public slots:
//    void setCurrentWorkspace(int index);
    void showNextWinbox();
    void showPrevWinbox();
    bool showIndicator();
    void hideIndicator();
    void hideIndicatorAndActivate(bool needActivate);
    bool handleWorkspace();
    bool showWorkspace();
    void hideWorkspace();

private:
    QHotkey *nextShortcut;
    QHotkey *prevShortcut;
    QHotkey *workspaceShortcut;
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
