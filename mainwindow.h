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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>

#include "mylabel.h"

#undef signals
#include <glib.h>

#define LINES 4			//Lines for displaying icons
#define COLS 5			//Icons for each line
#define TOP_SIZE 10		//Top reserved space
#define BOTTOM_SIZE 50		//Bottom reserved space, including that for title
#define LEFT_SIZE 10		//Right reserved space
#define RIGHT_SIZE 10		//Right reserved space
#define INTERVAL_WIDTH_SIZE 20  //Horizontal interval between two icons
#define INTERVAL_HEIGHT_SIZE 20 //Vertical interval between two icons

#define INTERVAL_TIME_MS 125

#define PREVIEW_WIDTH		168
#define PREVIEW_HEIGHT		128
#define SPACE_WIDTH			8
#define SPACE_HEIGHT		8
#define THUMBNAIL_WIDTH		(PREVIEW_WIDTH - SPACE_WIDTH)
#define THUMBNAIL_HEIGHT	(PREVIEW_HEIGHT - SPACE_HEIGHT)
#define ICON_WIDTH			48
#define ICON_HEIGHT			48

#define KEY_MAP_OFFSET_ALT_LEFT		8
#define KEY_MAP_OFFSET_ALT_RIGHT	13
#define KEY_MAP_MASK_ALT_LEFT		0x01
#define KEY_MAP_MASK_ALT_RIGHT		0x10

#define KEY_CHECK_INTERVAL_TIME_MS	50

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
	int WindowCount;
    QTimer altCheckTimer;
	GList *global_tab_list;

	QWidget *m_sub;
	QLabel *m_label;

	QList<MyLabel *> theLabels;
	QList<QString> theTitles;
	QList<QMainWindow *> screen_widget;
	int maxY[LINES];   	//Y for the highest icon each line
	int fontWidth, fontHeight;

	Ui::MainWindow *ui;

private slots:
	void show_forward();
	void show_backward();
	void slotMylabel(int index);
	void checkAltStatus();

private:
	void show_tab_list(int value);
	void hideWindow();
};

#endif // MAINWINDOW_H
