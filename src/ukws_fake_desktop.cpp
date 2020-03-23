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

#include "ukws_fake_desktop.h"

#include <QCoreApplication>
#include <QApplication>
#include <QScreen>
#include <QPainter>

UkwsFakeDesktop::UkwsFakeDesktop(QWidget *parent) : QWidget(parent)
{
    viewLabel = new QLabel();
    viewLabel->setScaledContents(true);

    mainLayout = new QVBoxLayout();
    mainLayout->addWidget(viewLabel);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setMargin(0);

    this->setLayout(mainLayout);
}

void UkwsFakeDesktop::setBackground(QPixmap background)
{
    this->background = background;
}

void UkwsFakeDesktop::reloadWindowList()
{
    if (wininfoList.size() != 0)
        cleanWininfo();

    wmOperator->updateWindowList();
    int size = wmOperator->windowQList->size();

    for (int i = 0; i < size; i++) {
        WnckWindow *win = wmOperator->windowQList->at(i);
        UkwsWindowInfo *wi = new UkwsWindowInfo(this);

        wi->setWnckWindow(win);
        wi->setOrigPixmapByWnck();
        wininfoList.append(wi);
    }
}

void UkwsFakeDesktop::reShow()
{
    reloadWindowList();
}

void UkwsFakeDesktop::reSetWindowThumbnailByWnck()
{
    QThread *workThread;
    UkwsWorker *worker;
    int i;

    // 获取CPU核数
    cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpus <= 0)
        cpus = 1;

    // 准备工作线程
    updateDesktopViewRequestId++;
    for (i = 0; i < cpus; i++) {
        workThread = new QThread;
        worker = new UkwsWorker(UkwsWorker::Winpixmap);

        // doingThread只是保存索引，无父子关系
        worker->doingThread = workThread;
        worker->cpu = i;
        worker->moveToThread(workThread);
        connect(workThread, &QThread::started, worker, &UkwsWorker::doWork);
        connect(worker, &UkwsWorker::workDone, workThread, &QThread::quit);
        connect(worker, &UkwsWorker::workDone, this, &UkwsFakeDesktop::doWorkerDone);
        workerList.append(worker);
    }

    // 分配工作线程
    UkwsWindowInfo *wi;
    int size = wininfoList.size();
    for (i = 0; i < size; i++) {
        wi = wininfoList.at(i);
        workerList[i % cpus]->appedWorkItem(wi);
    }

    // 开始处理
    for (i = 0; i < cpus; i++) {
        workerList.at(i)->doingThread->start();
    }

    // 不等待缩略图完全生成，直接进行之后的步骤
}

void UkwsFakeDesktop::cleanWininfo()
{
    // 等待处理完成，并在等待时处理其他事件
    UkwsWorker *worker;
    foreach(worker, workerList) {
        worker->stopWork();

        while (!worker->doingThread->isFinished()) {
            QCoreApplication::processEvents();
        }
        // 停止更新缩略图的线程
        worker->doingThread->quit();

        // doingThread只是保存索引，无父子关系，故需要手动释放
        worker->doingThread->deleteLater();
        worker->deleteLater();
    }
    workerList.clear();

    UkwsWindowInfo *wininfo;
    foreach(wininfo, wininfoList) {
        wininfo->deleteLater();
    }
    wininfoList.clear();
}

bool UkwsFakeDesktop::updateWindowViewPixmap(bool newRequest)
{
    UkwsWorker *worker;
    UkwsWindowInfo *wi;
    int i;
    bool allWorkDone = true;

    if (newRequest)
        updateDesktopViewRequestId++;

    // 已有任务在处理当前的视图，直接返回
    if (updateDesktopViewHandledId == updateDesktopViewRequestId)
        return false;

    for (i = 0; i < cpus; i++) {
        worker = workerList.at(i);
        if (!worker->isStopped())
            allWorkDone = false;
    }

    if (!allWorkDone)
        return false;

    // 所有任务完成（获取窗口截图并生成缩略图），开始合成桌面视图
    updateDesktopViewHandledId = updateDesktopViewRequestId;

    // 获取鼠标所在屏幕的尺寸
    QRect screenRect;
    int screenCount = QGuiApplication::screens().count();
    for (int i = 0  ; i < screenCount; i++) {
        screenRect = QGuiApplication::screens().at(i)->geometry();

        if (screenRect.contains(QCursor::pos()))
            break;
    }

    windowViewPixmap = QPixmap(screenRect.size());
    windowViewPixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&windowViewPixmap);
    for (i = wininfoList.size() - 1; i >= 0; i--) {
        wi = wininfoList.at(i);
        painter.drawPixmap(wi->winX, wi->winY, wi->winWidth, wi->winHeight, wi->origPixmap);
    }
    painter.end();

    // 合成桌面图片
    desktopViewPixmap = background.scaled(screenRect.size(), Qt::IgnoreAspectRatio,
                                          Qt::SmoothTransformation);
    painter.begin(&desktopViewPixmap);
    painter.drawPixmap(0, 0, screenRect.size().width(),
                       screenRect.size().height(), desktopViewPixmap);
    painter.end();

    viewLabel->setPixmap(desktopViewPixmap);
    emit windowViewPixmapChange(index);

    return true;
}

void UkwsFakeDesktop::doWorkerDone()
{
    updateWindowViewPixmap(false);
}

bool UkwsFakeDesktop::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    Q_UNUSED(event);

    return false;
}
