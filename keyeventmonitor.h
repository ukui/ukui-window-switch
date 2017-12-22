#ifndef KEYEVENTMONITOR_H
#define KEYEVENTMONITOR_H

#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>

class KeyEventMonitor : public QThread
{
	Q_OBJECT

  public:
	KeyEventMonitor(QObject *parent = 0);

  Q_SIGNALS:
	void KeyAltRelease();

  public Q_SLOTS:
	void isReleaseAlt(int code);

  protected:
	static void callback(XPointer trash, XRecordInterceptData *data);
	void handleRecordEvent(XRecordInterceptData *);
	void run();

  private:
	bool isRelease;
};

#endif // KEYEVENTMONITOR_H
