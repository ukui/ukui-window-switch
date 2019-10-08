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
#include <QTimer>
#include <QPainter>
#include <QtX11Extras/QX11Info>
#include <QDBusConnection>
#include <QDBusServiceWatcher>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#undef signals
extern "C" {
#include <alt_tab_list.h>
#include <ukwm/compositor/plugins/ukui_plugin.h>
}

char tab_list_image_file[PATH_MAX_LEN] = {0};
char workspace_image_file[PATH_MAX_LEN] = {0};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	QDBusServiceWatcher *watcher = new QDBusServiceWatcher(UKUI_PLUGIN_BUS_NAME,
														   QDBusConnection::sessionBus(),
														   QDBusServiceWatcher::WatchForOwnerChange,
														   this);
	QObject::connect(watcher,  &QDBusServiceWatcher::serviceOwnerChanged,
					 this, &MainWindow::onDBusNameOwnerChanged);

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
	if (sc == NULL or sc_u == NULL)
	{
		qFatal("Can not register shortcut, exit...");
		QApplication::exit(1);
	}

	connect(sc, SIGNAL(activated()), this, SLOT(show_forward()));
	connect(sc_u, SIGNAL(activated()), this, SLOT(show_backward()));

	connect(&(this->altCheckTimer), &QTimer::timeout, this, &MainWindow::checkAltStatus);
	altCheckTimer.setTimerType(Qt::CoarseTimer);
	altCheckTimer.setInterval(KEY_CHECK_INTERVAL_TIME_MS);

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

QPixmap qimageFromXImage(XImage* ximage)
{
	QImage::Format format = QImage::Format_ARGB32_Premultiplied;
	if (ximage->depth == 24)
		format = QImage::Format_RGB32;
	else if (ximage->depth == 16)
		format = QImage::Format_RGB16;

	QImage image = QImage(reinterpret_cast<uchar*>(ximage->data),
						  ximage->width, ximage->height,
						  ximage->bytes_per_line, format).copy();

	// Little Endian or Big Endian?
	if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && ximage->byte_order == MSBFirst)
			|| (QSysInfo::ByteOrder == QSysInfo::BigEndian && ximage->byte_order == LSBFirst)) {

		for (int i = 0; i < image.height(); i++) {
			if (ximage->depth == 16) {
				ushort* p = reinterpret_cast<ushort*>(image.scanLine(i));
				ushort* end = p + image.width();
				while (p < end) {
					*p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
					p++;
				}
			} else {
				uint* p = reinterpret_cast<uint*>(image.scanLine(i));
				uint* end = p + image.width();
				while (p < end) {
					*p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000)
						 | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
					p++;
				}
			}
		}
	}

	// Fix alpha channel
	if (format == QImage::Format_RGB32) {
		QRgb* p = reinterpret_cast<QRgb*>(image.bits());
		for (int y = 0; y < ximage->height; ++y) {
			for (int x = 0; x < ximage->width; ++x)
				p[x] |= 0xff000000;
			p += ximage->bytes_per_line / 4;
		}
	}

	return QPixmap::fromImage(image);
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

		int i;
		XImage *img = NULL;
		Display *display = NULL;
		XWindowAttributes attr;

		GList *tab_list = this->global_tab_list;
		display = XOpenDisplay(NULL);

		for (i = 0; i < this->WindowCount; i++)
		{
			int myline = i / COLS; 	//line is from 0
			//int mycol = i % COLS;   	//col is from 0

			//Produce icon
			alt_tab_item *ati = NULL;
			ati = (alt_tab_item *)g_list_nth_data(tab_list, i);
			theTitles.append(QString(ati->title_name));

			if (myline < LINES)
			{
				if (maxY[myline] < PREVIEW_HEIGHT)
					maxY[myline] = PREVIEW_HEIGHT;

				//Compute myx
				int j;
				int myx = LEFT_SIZE;
				for (j = myline * COLS; j < i; j++)
				{
					myx += theLabels[j]->width();
					myx += INTERVAL_WIDTH_SIZE;
				}

				//Compute myy
				int myy = TOP_SIZE;
				for (j = 0; j < myline; j++)
				{
					myy += maxY[j];
					myy += INTERVAL_HEIGHT_SIZE;
				}

				QPixmap pixmap;
				pixmap.load(tab_list_image_file);

				MyLabel *one = new MyLabel(i, ati->title_name, m_sub);
				one->move(myx, myy);
				one->resize(PREVIEW_WIDTH, PREVIEW_HEIGHT);

				XGetWindowAttributes(display, ati->xid, &attr);
				img = XGetImage(display, ati->xid, 0, 0,
								attr.width, attr.height, 0xffffffff,
								ZPixmap);

				QPixmap thumbnail = qimageFromXImage(img).scaled(THUMBNAIL_WIDTH,
																 THUMBNAIL_HEIGHT,
																 Qt::KeepAspectRatio,
																 Qt::FastTransformation);
				XDestroyImage(img);

				QPixmap icon = pixmap.copy(ati->x, ati->y, ati->width, ati->height).
							   scaled(ICON_WIDTH, ICON_HEIGHT,
									  Qt::KeepAspectRatio, Qt::FastTransformation);

				QPixmap preview(PREVIEW_WIDTH, PREVIEW_HEIGHT);
				preview.fill(Qt::transparent);

				QPainter painter(&preview);
				QRect source;
				QRect target;

				source = QRect(0, 0, thumbnail.width(), thumbnail.height());
				target = QRect((THUMBNAIL_WIDTH - thumbnail.width()) / 2,
							   (THUMBNAIL_HEIGHT - thumbnail.height()) / 2,
							   thumbnail.width(), thumbnail.height());
				painter.drawPixmap(target, thumbnail, source);

				source = QRect(0, 0, icon.width(), icon.height());
				target = QRect(PREVIEW_WIDTH -ICON_WIDTH,
							   PREVIEW_HEIGHT - ICON_HEIGHT,
							   icon.width(), icon.height());
				painter.drawPixmap(target, icon, source);

				painter.end();
				one->setPixmap(preview);

				theLabels.append(one);

//				connect(one, SIGNAL(myclicked(int)), this, SLOT(slotMylabel(int)));
			}
		}
		XCloseDisplay(display);

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
		}

		int max_height = TOP_SIZE;
		for (i = 0; i < lines; i++)
		{
			max_height += maxY[i];
			max_height += INTERVAL_HEIGHT_SIZE;
		}

		max_height = max_height - INTERVAL_HEIGHT_SIZE + BOTTOM_SIZE;

		m_sub->resize(max_width, max_height);

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
				screen_widget[i]->setGeometry(screen_rect);
				screen_widget[i]->showFullScreen();
			}
		}

		screen_rect = desktop->screenGeometry(ps_idx);
		this->move(screen_rect.x(), screen_rect.y());
		m_sub->move((screen_rect.width() - m_sub->width()) / 2,
					(screen_rect.height() - m_sub->height()) / 2);
	}

	//this->setWindowState(Qt::WindowActive);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
	this->activateWindow();
	this->showFullScreen();
	m_sub->show();

	fflush(stdout);

	int count = this->WindowCount;
	int index = this->WindowIndex;

	for (int i = 0; i < theLabels.size(); i++)
		theLabels[i]->setHideColor();

	index = (index + count + value) % count;
	this->WindowIndex = index;
	m_label->setText("");
	if (index < LINES * COLS)
	{
		theLabels[index]->setBrightColor();
	}
	m_label->setText(theTitles[index]); //All visible and invisible windows have their own titles.

	m_label->resize(m_sub->width() - 10, 20);
	m_label->move((m_sub->width() - m_label->width()) / 2, m_sub->height() - m_label->height() - 15);
	this->CanBeRelease = true;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Qt event handler is only responsible for handling the autorepeat event
	if (!event->isAutoRepeat())
		return;

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

void MainWindow::checkAltStatus()
{
	if (this->CanBeRelease == false)
		return;

	bool leftAltReleased = false;
	bool rightAltReleased = false;
	char keymap[32] = {0};
	static Display *display = NULL;

//	if (display == NULL)
	display = QX11Info::display();

	XQueryKeymap(display, keymap);

	if ((keymap[KEY_MAP_OFFSET_ALT_LEFT] & KEY_MAP_MASK_ALT_LEFT) == 0)
		leftAltReleased = true;

	if ((keymap[KEY_MAP_OFFSET_ALT_RIGHT] & KEY_MAP_MASK_ALT_RIGHT) == 0)
		rightAltReleased = true;

	if (leftAltReleased && rightAltReleased)
	{
		this->altCheckTimer.stop();
		this->hideWindow();
//		if (display != NULL)
//		{
//			display = NULL;
//		}
	}
}

void MainWindow::onDBusNameOwnerChanged(const QString &name,
										const QString &oldOwner,
										const QString &newOwner)
{
	Q_UNUSED(oldOwner);

	if (name == UKUI_PLUGIN_BUS_NAME) {
		if (newOwner.isEmpty()) {
			qDebug() << "ukwm dbus service status changed:"
					 << "inactivate";
			qApp->exit(0);
		} else {
			qDebug() << "ukwm dbus service status changed:"
					 << "activate";
		}
	}
}
