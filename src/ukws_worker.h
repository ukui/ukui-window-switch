#ifndef UKWS_WORKER_H
#define UKWS_WORKER_H

#include "ukws_window_box.h"

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
    explicit UkwsWorker(QObject *parent = nullptr);

    QList<UkwsWindowBox *> workList;
    int cpu;
    QThread *doingThread;

signals:
    void workDone();

public slots:
    void init();
    void doWork();
    void stopWork();

private:
    int status;
};

#endif // UKWS_WORKER_H
