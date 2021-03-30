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

#ifndef UKWS_STACK_BLUR_H
#define UKWS_STACK_BLUR_H

#include <QObject>
#include <QImage>
#include <QColor>
#include <QPoint>
#include <QTime>

class UkwsStackBlur : public QObject
{
    Q_OBJECT
public:
    explicit UkwsStackBlur();

    QImage origImage();
    QImage blurImage();
    QRect blurRect();
    int radius();
    int reduce();

    void setOrigImage(QImage origImage);
    void setBlurImage(QImage blurImage);
    void setBlurRect(QRect blurRect);
    void setRadius(int radius);
    void setReduce(int reduce);

public slots:
    void start();
    void calculate();

signals:
    void calculateStart();
    void calculateDone();

private:
    QImage m_origImage;
    QImage m_blurImage;

    QRect m_blurRect;

    int m_radius;
    int m_reduce;
};

#endif // UKWS_STACK_BLUR_H
