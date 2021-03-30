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

#include "ukws_stack_blur.h"

#include <QPainter>
#include <QDebug>

#define max(a, b)   ((a) > (b) ? (a) : (b))
#define min(a, b)   ((a) < (b) ? (a) : (b))

UkwsStackBlur::UkwsStackBlur()
{
    m_origImage = QImage(1, 1, QImage::Format_ARGB32);
    m_blurImage = QImage(1, 1, QImage::Format_ARGB32);
    m_radius = 3;
    m_reduce = 1;

    connect(this, &UkwsStackBlur::calculateStart, this, &UkwsStackBlur::calculate);
}

/*
 * 对行进行水平模糊（一行一行模糊，X轴上计算权重）
 * QImage srcImage 原图像
 * int radius 模糊核半径
 * int startX, int startY 起始点坐标
 * int endX, int endY 终止点坐标
 */
QImage doHorizontalBlur(QImage srcImage, int radius,
                        int startX, int startY, int endX, int endY)
{
    QImage newImage(srcImage);
    QColor pixelColor;

    // 防止缩放时边界点越界
    endX = min(endX, srcImage.rect().right());
    endY = min(endY, srcImage.rect().bottom());

    int xMax = srcImage.width() - 1; // 列最小值，为宽度-1
    int curX, curY, pickX, i;
    int rgbSum[3];

    /*
     * 计算总权重，用于归一化
     * r = radius + 1
     * 1 .. r..1
     * 1 .. r + r .. 1 - r
     * (1 + r) * r / 2 + (r + 1) * r / 2 - r
     * (1 + r) * r - r
     * r * r
     */
    int weightSum = (radius + 1) * (radius + 1);

    /*
     * 建立色阶与加权值的索引表
     * 每一种颜色有256个色阶
     * 权重总和为weightSum，权重最小粒度为1/weightSum，权重取值范围为：1/weightSum ~ 1
     * 所以索引表大小：256 * weightSum
     */
    short *weightedColorIndex = (short *) malloc(sizeof(short) * 256 * weightSum);
    for (i = 0; i < 256 * weightSum; i++) {
        weightedColorIndex[i] = (short) (i / weightSum);
    }

    int curRgb[3];      // 存储当前像素点的颜色值
    int rOuterRgb[3];   // 存储右侧卷积核外第一个像素点的颜色值
    int lInnerRgb[3];   // 存储左侧卷积核内第一个像素点的颜色值
    int nextRgb[3];     // 存储下一个像素点的颜色值

    int weight;     // 权重的分子部分
    int leftRSum, leftGSum, leftBSum;       // 左序列权重分子之和
    int rightRSum, rightGSum, rightBSum;    // 右序列权重分子之和
    int lInnerX, rOuterX;

    // 逐行计算加权值
    for (curY = startY; curY <= endY; curY++) {
        rightRSum = 0;
        rightGSum = 0;
        rightBSum = 0;
        leftRSum = 0;
        leftGSum = 0;
        leftBSum = 0;
        rgbSum[0] = 0;
        rgbSum[1] = 0;
        rgbSum[2] = 0;

        // 计算左序列权重分子之和、右序列权重分子之和、初始加权值
        for (i = -radius; i <= radius; i++) {
            // 开始点外的点的权值都由开始点继承
            pickX = max(startX, i + startX);
            // 结束点外的点的权值都由结束点继承
            pickX = min(pickX, endX);
            // 防止越界
            pickX = min(pickX, xMax);
            pixelColor = srcImage.pixel(pickX, curY);

            // 分离RGB
            curRgb[0] = pixelColor.red();
            curRgb[1] = pixelColor.green();
            curRgb[2] = pixelColor.blue();

            // 计算加权值，像素信息x权值，中间点(i==0)的权值系数为radius + 1
            weight = radius + 1 - abs(i);
            rgbSum[0] += curRgb[0] * weight;
            rgbSum[1] += curRgb[1] * weight;
            rgbSum[2] += curRgb[2] * weight;

            if (i <= 0) {
                // 构建左序列权重分子之和，中间点(i==0)算在左序列。注意，这里没有乘以系数，纯数列求和。
                leftRSum += curRgb[0];
                leftGSum += curRgb[1];
                leftBSum += curRgb[2];
            } else {
                // 构建右序列权重分子之和。注意，这里没有乘以系数，纯数列求和。
                rightRSum += curRgb[0];
                rightGSum += curRgb[1];
                rightBSum += curRgb[2];
            }
        }

        // 开始滑动卷积核，计算各个点的加权值
        for (curX = startX; curX <= endX; curX++) {
            // 使用计算好的加权值填充新图
            pixelColor.setRed(weightedColorIndex[rgbSum[0]]);
            pixelColor.setGreen(weightedColorIndex[rgbSum[1]]);
            pixelColor.setBlue(weightedColorIndex[rgbSum[2]]);
            newImage.setPixelColor(QPoint(curX, curY), pixelColor);

            // 计算卷积窗口左侧内部第一个点坐标
            lInnerX = curX - radius;
            // 防止越界
            lInnerX = max(lInnerX, 0);
            // 边界外点由边界代表
            lInnerX = max(lInnerX, startX);

            // 计算卷积窗口右侧外部第一个点坐标
            rOuterX = curX + radius + 1;
            // 防止越界
            rOuterX = min(rOuterX, xMax);
            // 边界外点由边界代表
            rOuterX = min(rOuterX, endX);

            // 提取下一个点的颜色，min函数放置越界
            pixelColor = srcImage.pixel(min(curX + 1, endX), curY);
            nextRgb[0] = pixelColor.red();
            nextRgb[1] = pixelColor.green();
            nextRgb[2] = pixelColor.blue();

            // 提取卷积窗口左侧内部第一个点的颜色
            pixelColor = srcImage.pixel(lInnerX, curY);
            lInnerRgb[0] = pixelColor.red();
            lInnerRgb[1] = pixelColor.green();
            lInnerRgb[2] = pixelColor.blue();

            // 提取卷积窗口右侧外部第一个点的颜色
            pixelColor = srcImage.pixel(rOuterX, curY);
            rOuterRgb[0] = pixelColor.red();
            rOuterRgb[1] = pixelColor.green();
            rOuterRgb[2] = pixelColor.blue();

            // 计算下个点的加权值
            rgbSum[0] = rgbSum[0] - leftRSum + rightRSum + rOuterRgb[0];
            rgbSum[1] = rgbSum[1] - leftGSum + rightGSum + rOuterRgb[1];
            rgbSum[2] = rgbSum[2] - leftBSum + rightBSum + rOuterRgb[2];

            // 计算下个点的左序列权重分子之和
            leftRSum = leftRSum - lInnerRgb[0] + nextRgb[0];
            leftGSum = leftGSum - lInnerRgb[1] + nextRgb[1];
            leftBSum = leftBSum - lInnerRgb[2] + nextRgb[2];

            // 计算下个点的右序列权重分子之和
            rightRSum = rightRSum - nextRgb[0] + rOuterRgb[0];
            rightGSum = rightGSum - nextRgb[1] + rOuterRgb[1];
            rightBSum = rightBSum - nextRgb[2] + rOuterRgb[2];
        }
        // 最后一个像素值已经在最后一轮循环计算完毕，这里直接填充
        pixelColor.setRed(weightedColorIndex[rgbSum[0]]);
        pixelColor.setGreen(weightedColorIndex[rgbSum[1]]);
        pixelColor.setBlue(weightedColorIndex[rgbSum[2]]);

        newImage.setPixelColor(QPoint(endX, curY), pixelColor);
    }
    free(weightedColorIndex);

    return newImage;
}

/*
 * 对列进垂直水模糊（一列一列模糊，Y轴上计算权重）
 * QImage srcImage 原图像
 * int radius 模糊核半径
 * int startX, int startY 起始点坐标
 * int endX, int endY 终止点坐标
 */
QImage doVerticalBlur(QImage srcImage, int radius,
                      int startX, int startY, int endX, int endY)
{
    QImage newImage(srcImage);
    QColor pixelColor;

    // 防止缩放时边界点越界
    endX = min(endX, srcImage.rect().right());
    endY = min(endY, srcImage.rect().bottom());

    int yMax = srcImage.height() - 1; // 列最小值，为宽度-1
    int curX, curY, pickY, i;
    int rgbSum[3];

    /*
     * 计算总权重，用于归一化
     * r = radius + 1
     * 1 .. r..1
     * 1 .. r + r .. 1 - r
     * (1 + r) * r / 2 + (r + 1) * r / 2 - r
     * (1 + r) * r - r
     * r * r
     */
    int weightSum = (radius + 1) * (radius + 1);

    /*
     * 建立色阶与加权值的索引表
     * 每一种颜色有256个色阶
     * 权重总和为weightSum，权重最小粒度为1/weightSum，权重取值范围为：1/weightSum ~ 1
     * 所以索引表大小：256 * weightSum
     */
    short *weightedColorIndex = (short *) malloc(sizeof(short) * 256 * weightSum);
    for (i = 0; i < 256 * weightSum; i++) {
        weightedColorIndex[i] = (short) (i / weightSum);
    }

    int curRgb[3];      // 存储当前像素点的颜色值
    int bOuterRgb[3];   // 存储卷积核底部外第一个像素点的颜色值
    int tInnerRgb[3];   // 存储卷积核上部内第一个像素点的颜色值
    int nextRgb[3];     // 存储下一个像素点的颜色值

    int weight;     // 权重的分子部分
    int topRSum, topGSum, topBSum;              // 上部权重分子之和
    int bottomRSum, bottomGSum, bottomBSum;     // 底部权重分子之和
    int tInnerY, bOuterY;

    // 逐列计算加权值
    for (curX = startX; curX <= endX; curX++) {
        bottomRSum = 0;
        bottomGSum = 0;
        bottomBSum = 0;
        topRSum = 0;
        topGSum = 0;
        topBSum = 0;
        rgbSum[0] = 0;
        rgbSum[1] = 0;
        rgbSum[2] = 0;

        // 计算左序列权重分子之和、右序列权重分子之和、初始加权值
        for (i = -radius; i <= radius; i++) {
            // 开始点外的点的权值都由开始点继承
            pickY = max(startY, i + startY);
            // 结束点外的点的权值都由结束点继承
            pickY = min(pickY, endY);
            // 防止越界
            pickY = min(pickY, yMax);
            pixelColor = srcImage.pixel(curX, pickY);

            // 分离RGB
            curRgb[0] = pixelColor.red();
            curRgb[1] = pixelColor.green();
            curRgb[2] = pixelColor.blue();

            // 计算加权值，像素信息x权值，中间点(i==0)的权值系数为radius + 1
            weight = radius + 1 - abs(i);
            rgbSum[0] += curRgb[0] * weight;
            rgbSum[1] += curRgb[1] * weight;
            rgbSum[2] += curRgb[2] * weight;

            if (i <= 0) {
                // 构建左序列权重分子之和，中间点(i==0)算在左序列。注意，这里没有乘以系数，纯数列求和。
                topRSum += curRgb[0];
                topGSum += curRgb[1];
                topBSum += curRgb[2];
            } else {
                // 构建右序列权重分子之和。注意，这里没有乘以系数，纯数列求和。
                bottomRSum += curRgb[0];
                bottomGSum += curRgb[1];
                bottomBSum += curRgb[2];
            }
        }

        // 开始滑动卷积核，计算各个点的加权值
        for (curY = startY; curY <= endY; curY++) {
            // 使用计算好的加权值填充新图
            pixelColor.setRed(weightedColorIndex[rgbSum[0]]);
            pixelColor.setGreen(weightedColorIndex[rgbSum[1]]);
            pixelColor.setBlue(weightedColorIndex[rgbSum[2]]);
            newImage.setPixelColor(QPoint(curX, curY), pixelColor);

            // 计算卷积窗口左侧内部第一个点坐标
            tInnerY = curY - radius;
            // 防止越界
            tInnerY = max(tInnerY, 0);
            // 边界外点由边界代表
            tInnerY = max(tInnerY, startY);

            // 计算卷积窗口右侧外部第一个点坐标
            bOuterY = curY + radius + 1;
            // 防止越界
            bOuterY = min(bOuterY, yMax);
            // 边界外点由边界代表
            bOuterY = min(bOuterY, endY);

            // 提取下一个点的颜色，min函数放置越界
            pixelColor = srcImage.pixel(curX, min(curY + 1, endY));
            nextRgb[0] = pixelColor.red();
            nextRgb[1] = pixelColor.green();
            nextRgb[2] = pixelColor.blue();

            // 提取卷积窗口左侧内部第一个点的颜色
            pixelColor = srcImage.pixel(curX, tInnerY);
            tInnerRgb[0] = pixelColor.red();
            tInnerRgb[1] = pixelColor.green();
            tInnerRgb[2] = pixelColor.blue();

            // 提取卷积窗口右侧外部第一个点的颜色
            pixelColor = srcImage.pixel(curX, bOuterY);
            bOuterRgb[0] = pixelColor.red();
            bOuterRgb[1] = pixelColor.green();
            bOuterRgb[2] = pixelColor.blue();

            // 计算下个点的加权值
            rgbSum[0] = rgbSum[0] - topRSum + bottomRSum + bOuterRgb[0];
            rgbSum[1] = rgbSum[1] - topGSum + bottomGSum + bOuterRgb[1];
            rgbSum[2] = rgbSum[2] - topBSum + bottomBSum + bOuterRgb[2];

            // 计算下个点的左序列权重分子之和
            topRSum = topRSum - tInnerRgb[0] + nextRgb[0];
            topGSum = topGSum - tInnerRgb[1] + nextRgb[1];
            topBSum = topBSum - tInnerRgb[2] + nextRgb[2];

            // 计算下个点的右序列权重分子之和
            bottomRSum = bottomRSum - nextRgb[0] + bOuterRgb[0];
            bottomGSum = bottomGSum - nextRgb[1] + bOuterRgb[1];
            bottomBSum = bottomBSum - nextRgb[2] + bOuterRgb[2];
        }
        // 最后一个像素值已经在最后一轮循环计算完毕，这里直接填充
        pixelColor.setRed(weightedColorIndex[rgbSum[0]]);
        pixelColor.setGreen(weightedColorIndex[rgbSum[1]]);
        pixelColor.setBlue(weightedColorIndex[rgbSum[2]]);

        newImage.setPixelColor(QPoint(curX, endY), pixelColor);
    }
    free(weightedColorIndex);

    return newImage;
}

void UkwsStackBlur::calculate()
{
    int h = m_origImage.height();

    // 缩小原图
    QImage blurImage = m_origImage.scaledToHeight(h / m_reduce,
                                                  Qt::FastTransformation);
    QRect rect;
    rect.setRect(m_blurRect.left() / m_reduce, m_blurRect.top() / m_reduce,
                 m_blurRect.width() / m_reduce, m_blurRect.height() / m_reduce);

    // 横向模糊
    blurImage = doHorizontalBlur(blurImage, m_radius,
                                 rect.left(), rect.top(),
                                 rect.right(), rect.bottom());
    // 纵向模糊
    blurImage = doVerticalBlur(blurImage, m_radius,
                               rect.left(), rect.top(),
                               rect.right(), rect.bottom());

    // 还原大小
    blurImage=  blurImage.scaledToHeight(h, Qt::FastTransformation);

    // 混合模糊区域与原图
    m_blurImage = m_origImage;
    QPainter painter;
    painter.begin(&m_blurImage);
    painter.drawImage(m_blurRect.topLeft(), blurImage, m_blurRect);
    painter.end();

    emit calculateDone();
}

QImage UkwsStackBlur::origImage()
{
    return m_origImage;
}

QImage UkwsStackBlur::blurImage()
{
    return m_blurImage;
}

QRect UkwsStackBlur::blurRect()
{
    return m_blurRect;
}

int UkwsStackBlur::radius()
{
    return m_radius;
}

int UkwsStackBlur::reduce()
{
    return m_reduce;
}

void UkwsStackBlur::setOrigImage(QImage origImage)
{
    m_origImage = origImage;
    // 更新模糊区域
    m_blurRect = origImage.rect();
}

void UkwsStackBlur::setBlurImage(QImage blurImage)
{
    m_blurImage = blurImage;
}

void UkwsStackBlur::setBlurRect(QRect blurRect)
{
    int blurX1, blurY1, blurX2, blurY2;
    int origX1, origY1, origX2, origY2;

    // 获取待模糊区域的尺寸信息
    blurX1 = blurRect.left();
    blurY1 = blurRect.top();
    blurX2 = blurRect.right();
    blurY2 = blurRect.bottom();

    // 获取原始图像的尺寸信息
    origX1 = m_origImage.rect().left();
    origY1 = m_origImage.rect().top();
    origX2 = m_origImage.rect().right();
    origY2 = m_origImage.rect().bottom();

    // 尺寸校准，模糊区域限定在原始图片内
    blurX1 = min(max(blurX1, origX1), origX2);
    blurY1 = min(max(blurY1, origY1), origY2);
    blurX2 = min(max(blurX2, origX1), origX2);
    blurY2 = min(max(blurY2, origY1), origY2);

    // 设置模糊区域信息
    m_blurRect.setTopLeft(QPoint(blurX1, blurY1));
    m_blurRect.setBottomRight(QPoint(blurX2, blurY2));
}

void UkwsStackBlur::setRadius(int radius)
{
    if (radius < 3)
        return;

    m_radius = radius;
}

void UkwsStackBlur::setReduce(int reduce)
{
    if (reduce < 1)
        return;

    m_reduce = reduce;
}

void UkwsStackBlur::start()
{
    emit calculateStart();
}
