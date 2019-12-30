#include "ukws_worker.h"

#include <sched.h>
#include <errno.h>

#include <QDebug>

UkwsWorker::UkwsWorker(QObject *parent) : QObject(parent)
{
    init();
    doingThread = nullptr;
}

void UkwsWorker::init()
{
    status = UkwsWorker::Stopped;
    cpu = 0;
    workList.clear();
}

void UkwsWorker::doWork()
{
    cpu_set_t mask;

//    qDebug() << "Worker" << cpu << "start";
    status = UkwsWorker::Running;

    // 设置CPU亲和力
    CPU_ZERO(&mask);    /* 初始化set集，将set置为空*/
    CPU_SET(cpu, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        qDebug("Worker %d set CPU affinity failue, ERROR:%s", cpu, strerror(errno));
    }

    // 开始处理
//    int size = workList.size();
    UkwsWindowBox *wb;
    foreach (wb, workList) {
        if (status == UkwsWorker::Running)
            wb->setThumbnailByWnck();
        else
            break;
    }

    // 设置状态
//    qDebug() << "Worker" << cpu << "done";
    status = UkwsWorker::Stopped;
    emit workDone();
}

void UkwsWorker::stopWork()
{
    status = UkwsWorker::Stopping;
}
