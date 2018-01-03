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

#include <QDesktopWidget>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qxtglobalshortcut.h"
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QDebug>
#include <QTime>

#include "keyeventmonitor.h"

#undef signals
extern "C" {
#include <alt_tab_list.h>
#include <ukwm/compositor/plugins/ukui_plugin.h>
}

char tab_list_image_file[PATH_MAX_LEN] = {0};
char workspace_image_file[PATH_MAX_LEN] = {0};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDesktopWidget *desktop = QApplication::desktop();
    move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);

    this->ShowStatus = false;
    this->CanBeRelease = true;
    for (int i = 0; i < LINES; i++)
	maxY[i] = 0;
    this->WindowIndex = 0;

    this->setAttribute(Qt::WA_TranslucentBackground, true);

    m_sub = new QWidget(this);
    m_sub->move((desktop->width() - this->width()) / 2, (desktop->height() - this->height()) / 2);

    m_label = new QLabel(m_sub);
    m_sub->setStyleSheet("background-color:rgba(0, 0, 0, 216);"
				"border-radius: 8px;"
				"border-color: rgb(200, 200, 200);"
				"border-style:solid;"
				"border-width:2px;");

    //QFont font("Arial", 30, QFont::Bold);
    QFont font("Microsoft YaHei", 10, 50); 
    m_label->setFont(font);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet("background-color: transparent;"
				"border: 0px; color: white");
    QFontMetrics fm(font);
    fontWidth = fm.maxWidth();
    fontHeight = fm.height();

    QxtGlobalShortcut *sc = new QxtGlobalShortcut(QKeySequence("Alt+Tab"), this);
    QxtGlobalShortcut *sc_u = new QxtGlobalShortcut(QKeySequence("Alt+Shift+Tab"), this);
    connect(sc, SIGNAL(activated()), this, SLOT(show_forward()));
    connect(sc_u, SIGNAL(activated()), this, SLOT(show_backward()));

    KeyEventMonitor *kem = new KeyEventMonitor;
    connect(kem, SIGNAL(KeyAltRelease()), this, SLOT(doAltRelease()));
    kem->start();
    //QWidget::installEventFilter(this); //Install event filter for this window.

    snprintf(tab_list_image_file, PATH_MAX_LEN, "/run/user/%d/%s",
			 getuid(), TAB_LIST_IMAGE_FILE);
}

MainWindow::~MainWindow()
{
    for (int i = 0; i < theLabels.size(); i++)
        delete theLabels[i];
    theLabels.clear();
    theTitles.clear();

    delete m_label;
    delete m_sub;
    delete ui;
}

void MainWindow::show_forward()
{
    show_tab_list(1);
}

void MainWindow::show_backward()
{
    show_tab_list(-1);
}

void MainWindow::show_tab_list(int value)
{
    static unsigned long tabPreviousTime = 0;
    unsigned long tabCurrentTime = 0;

    tabCurrentTime = QDateTime::currentMSecsSinceEpoch();
    if (tabCurrentTime - tabPreviousTime < INTERVAL_TIME_MS)
    {
	return;
    }
    tabPreviousTime = tabCurrentTime;

    this->CanBeRelease = false;

    if (!this->ShowStatus)
    {
	for (int i = 0; i < LINES; i++)
	    maxY[i] = 0;
	theLabels.clear();

	InitUkwmPluginDBusComm();
	while (!UkwmPluginDBusServiceIsReady())
	{
	    usleep(1000);
	}

	this->global_tab_list = DBusGetAltTabList();
	this->WindowCount = g_list_length(this->global_tab_list);
	FinishUkwmPluginDBusComm();

	if (this->WindowCount == 0)
	{
	    this->CanBeRelease = true;
	    return;
	}

	this->ShowStatus = true;

	//Generate min{20, this->WindowCount} labels
	int total = LINES * COLS;
	int n_labels = total; //How many labels will be showed?
	if (total > this->WindowCount)
	n_labels = this->WindowCount;
	int lines;
	lines = n_labels / COLS;
	if (lines * COLS < n_labels)
	    lines++;

	//qDebug() << "total=" << total << "n_labels=" << n_labels << "lines=" << lines;

	GList *tab_list = this->global_tab_list;
	int i;
	for (i = 0; i < this->WindowCount; i++)
	{
	    int myline = i / COLS; 	//line is from 0
	    //int mycol = i % COLS;   	//col is from 0

	    //qDebug() << "myline: " << myline << "mycol: " << mycol;

	    //Produce icon
	    alt_tab_item *ati = NULL;
	    ati = (alt_tab_item *)g_list_nth_data(tab_list, i);
	    theTitles.append(QString(ati->title_name));

	    if (myline < LINES)
	    {
		if (maxY[myline] < ati->height)
			maxY[myline] = ati->height;

		//Compute myx
		int j;
		int myx = LEFT_SIZE;
		for (j = myline * COLS; j < i; j++)
		{
		    myx += theLabels[j]->width();
		    myx += INTERVAL_WIDTH_SIZE;
		}
		//qDebug() << "Compute line " << i << "'s x，from " << (myline*COLS) << " to " << i << "==" << myx;

		//Compute myy
		int myy = TOP_SIZE;
		for (j = 0; j < myline; j++)
		{
		    myy += maxY[j];
		    myy += INTERVAL_HEIGHT_SIZE;
		}
		//qDebug() << "Compute col " << i << "'s y，from 0 to " << mycol << "==" << myy;

		QPixmap pixmap;
		pixmap.load(tab_list_image_file);

		MyLabel *one = new MyLabel(i, ati->title_name, m_sub);
		//qDebug() << "x, y, width, height" << ati->x << ati->y << ati->width << ati->height;
		one->setPixmap(pixmap.copy(ati->x + 2, ati->y, ati->width, ati->height));
				one->move(myx, myy);
				one->resize(ati->width, ati->height);
				theLabels.append(one);

		connect(one, SIGNAL(myclicked(int)), this, SLOT(slotMylabel(int)));
	    }
	}

	//Compute the size (wx, wy) of the whole window.
	int j;
	int max_width = 0;
	for (i = 0; i < lines; i++)
	{
	    int wx = LEFT_SIZE;
	    for (j = 0; j < COLS; j++)
	    {
	        if ((i * COLS + j) >= n_labels)
		    break;

		wx += theLabels[i * COLS + j]->width();
		wx += INTERVAL_HEIGHT_SIZE;
	    }

	    wx = wx - INTERVAL_HEIGHT_SIZE + RIGHT_SIZE;

	    if (max_width < wx)
		max_width = wx;
	    //qDebug() << "line " << i << "'s width is " << wx;
	}

	//qDebug() << "The window's width is " << max_width;

	int max_height = TOP_SIZE;
	for (i = 0; i < lines; i++)
	{
	    max_height += maxY[i];
	    max_height += INTERVAL_HEIGHT_SIZE;
	}

	max_height = max_height - INTERVAL_HEIGHT_SIZE + BOTTOM_SIZE;
	//qDebug() << "The window's height is " << max_height;

	m_sub->resize(max_width, max_height);
	//QDesktopWidget* desktop = QApplication::desktop();
	//m_sub->move((desktop->width() - m_sub->width()) / 2,
	//		(desktop->height() - m_sub->height()) / 2);

	QDesktopWidget *desktop = QApplication::desktop();
	QRect screen_rect;

	int s_num = desktop->screenCount();
	int ps_idx = desktop->primaryScreen();

	for (i = 0; i < screen_widget.length(); i++)
	{
 	    screen_widget[i]->hide();
	}

	//For multiple screens, use transparent QMainWindow to prevent clicking the mouse in other screens.
	for (i = 0; i < s_num; i++)
	{
	    if ((i + 1) > screen_widget.length())
	    {
		//qDebug() << "New QRect and QMainWindow";
		screen_widget.append(new QMainWindow(this));
		screen_widget[i]->setWindowFlags(Qt::FramelessWindowHint |
						 Qt::WindowStaysOnTopHint |
						 Qt::Tool);
		screen_widget[i]->setWindowState(Qt::WindowActive);
		screen_widget[i]->setAttribute(Qt::WA_TranslucentBackground);
		screen_widget[i]->setWindowOpacity(0);
	    }

	    if (i != ps_idx)
	    {
		screen_rect = desktop->screenGeometry(i);
		//qDebug() << "Screen " << i << ": " << screen_rect;
		screen_widget[i]->setGeometry(screen_rect);
		screen_widget[i]->showFullScreen();
	    }
	}

	screen_rect = desktop->screenGeometry(ps_idx);
	//qDebug() << "Primary Screen Rect = " << screen_rect;
	this->move(screen_rect.x(), screen_rect.y());
	m_sub->move((screen_rect.width() - m_sub->width()) / 2,
			(screen_rect.height() - m_sub->height()) / 2);
	//qDebug() << "After move, this window rect = " << this->rect();
    }

    //this->setWindowState(Qt::WindowActive);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    this->activateWindow();
    this->showFullScreen();
    m_sub->show();

    fflush(stdout);

    int count = this->WindowCount;
    int index = this->WindowIndex;
    //qDebug() << "count: " << count << "index: " << index << "theLabels.size()=" << theLabels.size();
    for (int i = 0; i < theLabels.size(); i++)
	theLabels[i]->setHideColor();

    index = (index + count + value) % count;
    this->WindowIndex = index;
    m_label->setText("");
    if (index < LINES * COLS)
    {
	theLabels[index]->setBrightColor();
	//m_label->setText(theLabels[index]->getTitle()); //Only LINES*COLS windows have their own titles.
    }
    m_label->setText(theTitles[index]); //All visible and invisible windows have their own titles.

    m_label->resize(m_sub->width() - 10, 20);
    m_label->move((m_sub->width() - m_label->width()) / 2, m_sub->height() - m_label->height() - 15);
    this->CanBeRelease = true;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    unsigned int modifiers = event->modifiers() & Qt::KeyboardModifierMask;

    if ((event->key() == Qt::Key_Tab) || (event->key() == Qt::Key_Backtab))
    {
	if (modifiers == Qt::AltModifier)
	    show_tab_list(1);

	if (modifiers == (Qt::ShiftModifier | Qt::AltModifier))
	    show_tab_list(-1);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

void MainWindow::slotMylabel(int index)
{
    //qDebug() << "clicked label" << index;
    for (int i = 0; i < theLabels.size(); i++)
	theLabels[i]->setHideColor();

    this->WindowIndex = index;

    hideWindow();
}

void MainWindow::hideWindow()
{
    if (this->ShowStatus)
    {
	int i = 0;
	for (i = 0; i < theLabels.size(); i++)
 	    delete theLabels[i];
	theLabels.clear();
	theTitles.clear();

	for (i = 0; i < screen_widget.length(); i++)
	    screen_widget[i]->hide();

	InitUkwmPluginDBusComm();
	DBusActivateWindowByTabListIndex(this->WindowIndex);
	FinishUkwmPluginDBusComm();

	this->hide();
	this->ShowStatus = false;
	this->WindowIndex = 0;
    }
}

void MainWindow::doAltRelease()
{
    int times = 0;
    while ((!this->CanBeRelease) && (times <= 10))
    {
	times++;
	usleep(100 * 1000);
    }
    this->hideWindow();
}
