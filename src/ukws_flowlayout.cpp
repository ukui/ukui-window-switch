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

#include <QtWidgets>
#include <QDebug>

#include "ukws_flowlayout.h"
UkwsFlowLayout::UkwsFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    m_rectList.clear();
    maxWidth = 0;
    maxHeight = 0;
    setContentsMargins(margin, margin, margin, margin);
}

UkwsFlowLayout::UkwsFlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    maxWidth = 0;
    maxHeight = 0;
    setContentsMargins(margin, margin, margin, margin);
}

UkwsFlowLayout::~UkwsFlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void UkwsFlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

int UkwsFlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int UkwsFlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int UkwsFlowLayout::count() const
{
    qDebug() << "the size of itemList: " << itemList.size();
    return itemList.size();
}

QLayoutItem *UkwsFlowLayout::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem *UkwsFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations UkwsFlowLayout::expandingDirections() const
{
    return 0;
}

bool UkwsFlowLayout::hasHeightForWidth() const
{
    return true;
}

int UkwsFlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);

    return height;
}

void UkwsFlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize UkwsFlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize UkwsFlowLayout::minimumSize() const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, itemList)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2*margin(), 2*margin());
    return size;
}

int UkwsFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    QList<QRect *> rectList;
    QList<int> spaceXList;
    QList<int> spaceYList;
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;
    int layoutWidth = 0;
    int layoutHeight = 0;

    m_rectList.clear();
    QLayoutItem *item;
    foreach (item, itemList) {
        QWidget *wid = item->widget();
        QRect *itemRect = new QRect;

        *itemRect = item->geometry();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        rectList.append(itemRect);
        spaceXList.append(spaceX);
        spaceYList.append(spaceY);

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        m_rectList.append(QRect(QPoint(x, y), item->sizeHint()));

        if (x + item->sizeHint().width() > layoutWidth)
            layoutWidth = x + item->sizeHint().width();

        if ((y + item->sizeHint().height()) > layoutHeight)
            layoutHeight = y + item->sizeHint().height();

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    if (!testOnly) {
        maxWidth = layoutWidth;
        maxHeight = layoutHeight;
    }

    int index;

    // 如果是水平方向对齐不是左对齐，则需修复对齐（只考虑左对齐和居中对齐）
    // 水平对齐掩码0x0f，左对齐01，右对齐02，居中对齐04，两端对齐08
    int layoutHAlign = this->alignment() & 0x0f;
    if (layoutHAlign != Qt::AlignLeft) {
        // 开始水平对齐修复
        if (itemList.size() > 0) {
            // FlowLayout中有控件
            int lineStartIndex = 0; // 行起始索引
            int lineEndIndex = 0;   // 行终止索引
            int lineY = 0;          // 当前行的顶部Y轴坐标，顶部Y轴坐标一致的item视作一行
            foreach (item, itemList) {
                index = itemList.indexOf(item);
                QRect rect = m_rectList.at(index);
                if (rect.y() != lineY) {
                    // 顶部Y轴坐标不一致，表示换行了
                    lineEndIndex = itemList.indexOf(item) - 1;
                    alignHFix(lineStartIndex, lineEndIndex, effectiveRect);

                    // 更新lineStartIndex为当前控件索引，更新当前行的顶部Y轴坐标
                    lineStartIndex = itemList.indexOf(item);
                    lineY = rect.y();
                }
            }

            // 处理最后一行
            lineEndIndex = itemList.count() - 1;
            alignHFix(lineStartIndex, lineEndIndex, effectiveRect);
            lastLineSpace = effectiveRect.right() - effectiveRect.left()
                    - (m_rectList.at(lineEndIndex).right()
                       - m_rectList.at(lineStartIndex).left());
        }
    }

    // 如果是垂直方向对齐不是顶对齐，则需修复对齐（只考虑顶对齐和居中对齐）
    // 垂直对齐掩码0xf0，顶对齐20，底对齐40，居中对齐80
    int layoutVAlign = this->alignment() & 0xf0;

    if ((layoutVAlign != Qt::AlignTop) && (layoutHeight < visualHeight)) {
        // 开始垂直对齐修复
        if (itemList.size() > 0) {
            // FlowLayout中有控件
            alignVFix(visualHeight - layoutHeight, effectiveRect);
        }
    }

    // 如非test，移动item
    if (!testOnly) {
        for (int i = 0; i < itemList.count(); i++) {
            itemList[i]->setGeometry(m_rectList[i]);
        }
    }

    return y + lineHeight - rect.y() + bottom;
}

int UkwsFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

void UkwsFlowLayout::alignHFix(int startIndex, int endIndex, QRect effectiveRect) const
{
    // 计算偏移量
    int width = m_rectList.at(endIndex).right() - m_rectList.at(startIndex).left();
    int rightSpace = effectiveRect.right() - effectiveRect.left() - width;
    int offset = 0;

    if (rightSpace > 0) {
        // 水平对齐掩码0x0f
        int layoutHAlign = this->alignment() & 0x0f;
        if (layoutHAlign == Qt::AlignHCenter)
            offset = rightSpace / 2;
        else
            offset = rightSpace;
    }

    // 移动item
    QLayoutItem *item;
    int fixedX = effectiveRect.x() + offset;
    int fixedY = m_rectList.at(startIndex).top();
    for (int i = startIndex; i <= endIndex; i++) {
        item = itemList.at(i);
        QWidget *wid = item->widget();

        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);

        m_rectList[i].setRect(fixedX, fixedY, item->sizeHint().width(), item->sizeHint().height());
        fixedX = fixedX + item->sizeHint().width() + spaceX;
    }
}

void UkwsFlowLayout::alignVFix(int blank, QRect effectiveRect) const
{
    Q_UNUSED(effectiveRect);

    int offset = 0;
    int y = 0;
    int size = m_rectList.size();
    int layoutVAlign = this->alignment() & 0xf0;

    // 中心对齐，只移动一般的空白
    if (layoutVAlign == Qt::AlignVCenter)
        offset = blank / 2;

    // 底对齐，移动所有的空白
    if (layoutVAlign == Qt::AlignBottom)
        offset = blank;

    // 重新设置item关联的区域位置，移动交由最后步骤完成
    QLayoutItem *item;
    for (int i = 0; i < size; i++) {
        item = itemList.at(i);
        y = m_rectList[i].y();
        m_rectList[i].setY(y + offset);
        item->setGeometry(QRect(QPoint(m_rectList[i].x(), m_rectList[i].y()), item->sizeHint()));
    }
}
