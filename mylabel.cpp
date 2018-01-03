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

#include "mylabel.h"

MyLabel::MyLabel(int index, const QString &text, QWidget *parent)
	: QLabel(parent)
{
    //setText(text);
    m_index = index;
    m_title = text;
}

MyLabel::~MyLabel()
{
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
	emit myclicked(m_index);
	setBrightColor();
    }

    QLabel::mousePressEvent(event); //deliver the event to parent class.
}

void MyLabel::setBrightColor()
{
    setStyleSheet("background-color: transparent; border: 2px solid white");
}

void MyLabel::setHideColor()
{
    setStyleSheet("background-color: transparent; border: 2px solid transparent");
}

QString MyLabel::getTitle()
{
    return m_title;
}

void MyLabel::setTitle(QString title)
{
    m_title = title;
}

int MyLabel::getIndex()
{
    return m_index;
}

void MyLabel::setIndex(int index)
{
    m_index = index;
}
