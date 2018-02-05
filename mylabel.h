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

#ifndef MY_LABEL_H
#define MY_LABEL_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class MyLabel : public QLabel
{
	Q_OBJECT
public:
	explicit MyLabel(int index, const QString &text = "", QWidget *parent = 0);
	~MyLabel();
signals:
	void myclicked(int);

protected:
	void mousePressEvent(QMouseEvent *event);

public:
	void setBrightColor();
	void setHideColor();
	QString getTitle();
	void setTitle(QString title);
	int getIndex();
	void setIndex(int index);

private:
	int m_index;
	QString m_title;
};

#endif //MY_LABEL_H
