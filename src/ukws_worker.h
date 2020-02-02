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

#ifndef UKWS_WORKER_H
#define UKWS_WORKER_H

#include "ukws_window_box.h"
#include "ukws_window_info.h"

#include <QObject>
#include <QThread>

class UkwsWorker : public QObject
{
    enum UkwsWorkerStatus {
        Running,
        Stopping,
        Stopped,
    };

    Q_OBJECT

public:
    enum UkwsWorkerType {
        Winbox,
        Winpixmap,
    };

    explicit UkwsWorker(UkwsWorkerType type, QObject *parent = nullptr);
    void appedWorkItem(UkwsWindowBox *winbox);
    void appedWorkItem(UkwsWindowInfo *wininfo);
    bool isStopped();

    QList<UkwsWindowBox *> winboxList;
    QList<UkwsWindowInfo *> wininfoList;
    int cpu;
    QThread *doingThread;

signals:
    void workDone();

public slots:
    void init();
    void doWork();
    void stopWork();

private:
    UkwsWorkerType workerType;
    int status;
};

#endif // UKWS_WORKER_H
