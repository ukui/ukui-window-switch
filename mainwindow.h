#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include "mylabel.h"

#define LINES 4			//Lines for displaying icons
#define COLS 5			//Icons for each line
#define TOP_SIZE 10		//Top reserved space
#define BOTTOM_SIZE 50		//Bottom reserved space, including that for title
#define LEFT_SIZE 10		//Right reserved space
#define RIGHT_SIZE 10		//Right reserved space
#define INTERVAL_WIDTH_SIZE 20  //Horizontal interval between two icons
#define INTERVAL_HEIGHT_SIZE 20 //Vertical interval between two icons

#define INTERVAL_TIME_MS 125

#undef signals
#include <glib.h>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

  protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

  private:
	bool ShowStatus;
	bool CanBeRelease;
	int WindowIndex;
	GList *global_tab_list;
	int WindowCount;

	QWidget *m_sub;
	QLabel *m_label;

	QList<MyLabel *> theLabels;
	QList<QString> theTitles;
	QList<QMainWindow *> screen_widget;
	int maxY[LINES];   	//Y for the highest icon each line
	int fontWidth, fontHeight;

	Ui::MainWindow *ui;

  private slots:
	void show_tab_list(int value);
	void show_forward();
	void show_backward();
	void slotMylabel(int index);
	void doAltRelease();

  private:
	void hideWindow();
};

#endif // MAINWINDOW_H
