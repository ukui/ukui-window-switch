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

#include "ukws_wnck_operator.h"
#include "ukws_window_box.h"
#include "ukws_indicator.h"
#include "ukws_workspace_manager.h"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QScreen>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QFileInfo>
#include <QDate>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStyleFactory>

#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <pwd.h>
#include <gdk/gdk.h>

#include "ukws_helper.h"
#include "ukws_config.h"
#include "ukws_dbus.h"
#include "ukws_manager.h"

#ifndef UKWS_DATA_DEFAULT_DIR
#define UKWS_DATA_DEFAULT_DIR "/usr/share/ukui-window-switch/"
#endif

#define PROGRAM_NAME "ukui-window-switch"
#define PATH_MAX_LEN 1024
#define PID_STRING_LEN 64

int checkProcessRunning(const char *processName)
{
    int uid = getuid();
    int pid = getpid();

    char pid_file[PATH_MAX_LEN] = {0};
    char pid_string[PID_STRING_LEN] = {0};
    char *display = NULL;

    display = getenv("DISPLAY");
    snprintf(pid_file, PATH_MAX_LEN, "/run/user/%d/%s_%s.pid", uid, processName, display);

    int pid_file_fd = open(pid_file, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (pid_file_fd < 0)
    {
        fprintf(stderr, "Can not open pid file: %s\n", pid_file);
        return -1;
    }

    int lock_ret = flock(pid_file_fd, LOCK_EX | LOCK_NB);
    if (lock_ret < 0)
    {
        struct passwd *pwd = getpwuid(uid);
        fprintf(stdout, "User %s[%d] has run %s, the current program exits.\n",
                pwd->pw_name, uid, processName);
        return 1;
    }

    snprintf(pid_string, PID_STRING_LEN, "%d\n", pid);
    write(pid_file_fd, pid_string, strlen(pid_string));
    fsync(pid_file_fd);

    return 0;
}

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static FILE *fp = NULL;
    static char logPath[255] = {0};
    static int uid = -1;

    Q_UNUSED(context);

    // 初始执行时，设置log文件路径
    if (uid == -1) {
        uid = getuid();
        sprintf(logPath, "/run/user/%d/%s.log", uid, PROGRAM_NAME);
    }

    if (access(logPath, F_OK|W_OK) == 0) {
        // log文件存在且可写
        if (fp == NULL) {
            fp = fopen(logPath, "a+");
        }
    } else {
        if (fp != NULL)
            fclose(fp);
        fp = NULL;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yy.MM.dd hh:mm:ss +zzz");

    // 获取用于控制命令行输出的环境变量
    char *ctrlEnv = getenv("UKWS_DEBUG");
    QString env;

    QString outMsg;
    switch (type) {
    case QtDebugMsg:
        outMsg = QString("[%1 D]: %2").arg(timeStr).arg(msg);
        break;
    case QtInfoMsg:
        outMsg = QString("[%1 I]: %2").arg(timeStr).arg(msg);
        break;
    case QtWarningMsg:
        outMsg = QString("[%1 W]: %2").arg(timeStr).arg(msg);
        break;
    case QtCriticalMsg:
        outMsg = QString("[%1 C]: %2").arg(timeStr).arg(msg);
        break;
    case QtFatalMsg:
        outMsg = QString("[%1 F]: %2").arg(timeStr).arg(msg);
    }

    if (fp != NULL) {
        fprintf(fp, "%s\n", outMsg.toUtf8().data());
        fflush(fp);
    }

    if (ctrlEnv != NULL) {
        // 环境变量为true或者1，则将信息输出到命令行
        env = QString(ctrlEnv).toLower();
        if ((env == "true") || (env == "1")) {
            fprintf(stdout, "%s\n", outMsg.toStdString().c_str());
            fflush(stdout);
        }
    }

    // 遇到致命错误，需要终止程序
    if (type == QtFatalMsg)
        abort();
}

void handleWorkspaceView()
{
    QString object = QString(getenv("DISPLAY"));
    object = object.trimmed().replace(":", "_").replace(".", "_").replace("-", "_");
    object = "/org/ukui/WindowSwitch/display/" + object;
    QDBusInterface interface("org.ukui.WindowSwitch", object,
                                "org.ukui.WindowSwitch",
                                QDBusConnection::sessionBus());
    if (!interface.isValid()) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    //调用远程的value方法
    QDBusReply<bool> reply = interface.call("handleWorkspace");
    if (reply.isValid()) {
        if (!reply.value())
            qWarning() << "Handle Workspace View Failed";
    } else {
        qCritical() << "Call Dbus method failed";
    }
}

void reloadConfig()
{
    QDBusInterface interface("org.ukui.WindowSwitch", "/org/ukui/WindowSwitch",
                                "org.ukui.WindowSwitch",
                                QDBusConnection::sessionBus());
    if (!interface.isValid()) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    //调用远程的value方法
    QDBusReply<bool> reply = interface.call("reloadConfig");
    if (reply.isValid()) {
        if (!reply.value())
            qWarning() << "Reload Config Failed";
    } else {
        qCritical() << "Call Dbus method failed";
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(msgHandler);

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication a(argc, argv);

    // libwnck使用GDK的接口，需要使用gdk_init来初始化GDK
    gdk_init (&argc, &argv);

    QCommandLineParser parser;
    QCommandLineOption showWorkspaceOption("show-workspace", "show or hide workspace view");
    QCommandLineOption reloadConfigOption("reload", "reload config");
    parser.addOption(showWorkspaceOption);
    parser.addOption(reloadConfigOption);
    parser.process(a);

    if (parser.isSet("show-workspace")) {
        handleWorkspaceView();
        return 0;
    }

    if (parser.isSet("reload")) {
        reloadConfig();
        return 0;
    }

    //Check if another process is running.
    int checkRet = checkProcessRunning(PROGRAM_NAME);
    if (checkRet != 0) {
        return checkRet;
    }

    UkwsDbusWatcher dbusWatcher;

    UkwsConfig *conf = new UkwsConfig;
    conf->setConfigFile("ukui-window-switch.conf", UKWS_CONF_DEFAULT_DIR);
    conf->configReload();

    UkwsManager winManager;
    winManager.setConfig(conf);

    return a.exec();
}
