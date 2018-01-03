/*
 * Copyright (C) 2017 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "mainwindow.h"
#include <QApplication>
#include <unistd.h>
#include "qxtglobalshortcut.h"
#include <QDebug>

#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <pwd.h>

#define PROGRAM_NAME "ukui-window-switch"
#define PATH_MAX_LEN 1024
#define PID_STRING_LEN 64

#define SHORTCAT_REG_DELAY 6

/*
 * Check if another process is running.
 * Input: program name	
 * Return:
 * 	0	No another process is running.
 * 	1	Another process is running.
 * 	<0	Exception.
 */
int checkProcessRunning(const char *processName)
{
    int uid = getuid();
    int pid = getpid();

    char pid_file[PATH_MAX_LEN] = {0};
    char pid_string[PID_STRING_LEN] = {0};

    snprintf(pid_file, PATH_MAX_LEN, "/run/user/%d/%s.pid", uid, processName);
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

int main(int argc, char *argv[])
{
    //Check if another process is running.
    int check_ret = checkProcessRunning(PROGRAM_NAME);
    if (check_ret != 0)
    {
	return check_ret;
    }

    //sleep(SHORTCAT_REG_DELAY);

    QApplication a(argc, argv);
    QFile qss(":/style/global");
    QString locale = QLocale::system().name();

    MainWindow w;
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    w.setWindowState(Qt::WindowActive);
    //w.setAttribute(Qt::WA_TranslucentBackground, false);
    //w.setWindowOpacity(0.5);

    return a.exec();
}
