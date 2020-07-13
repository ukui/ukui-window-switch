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

#include "ukws_common.h"
#include "ukws_workspace_box.h"
#include "ukws_workspace_manager.h"

#include <QDebug>
#include <QScrollArea>
#include <QtX11Extras/QX11Info>
#include <QScreen>
#include <QApplication>
#include <QScreen>
#include <QStyleOption>
#include <QPainter>
#include <QX11Info>

#include "ukws_stack_blur.h"

// ukws_helper.h包含X.h，必须放到所有Qt头文件之前
#include "ukws_helper.h"

UkwsWorkspaceManager::UkwsWorkspaceManager(QWidget *parent) : QWidget(parent)
{
    wmOperator = new UkwsWnckOperator;
    indStack = new QStackedWidget;
    newWorkspaceBtn = new UkwsNewWorkspaceButton;

    mainLayout = new QHBoxLayout;
    wsboxLayout = new QVBoxLayout;
    topSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed);

    showStatus = UkwsWidgetShowStatus::Hidden;
    config = nullptr;

    mainLayout->addWidget(indStack);
    mainLayout->addLayout(wsboxLayout);
    wsboxLayout->addSpacerItem(topSpacer);

    mainLayout->setAlignment(Qt::AlignHCenter);
    wsboxLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    mainLayout->setMargin(0);
    wsboxLayout->setMargin(0);

    this->setLayout(mainLayout);
    this->resize(1200, 600);

    this->installEventFilter(this);
}

void UkwsWorkspaceManager::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

    QWidget::paintEvent(event);
}

void UkwsWorkspaceManager::reloadWorkspace(int minScale)
{
    cleanAllWorkspace();

    // 获取主屏区域信息
    QList<QScreen *> screenList = QGuiApplication::screens();
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QRect screenRect = primaryScreen->geometry();
    int screenNum = screenList.indexOf(primaryScreen);
    WnckScreen *screen = wnck_screen_get(screenNum);

    float scale = (float)config->workspaceItemUnits / config->workspaceAllUnits;

    int w = screenRect.width() * scale;
    int h = screenRect.height() * scale;

    this->resize(screenRect.width(), screenRect.height());

    wmOperator->updateWorkspaceList();
    int size = wmOperator->workspaceQList->size();

    // 获取背景图片
    this->getBackground();

    // 设置工作区视图的最底层背景
    this->setBackgroundImage(screenRect.width(), screenRect.height());

    topSpacer->changeSize(w, screenRect.height() / 40);

    QPixmap wsboxBackground = background.scaled(QSize(w, h),
                                                Qt::IgnoreAspectRatio,
                                                Qt::SmoothTransformation);

    for (int i = 0; i < size; i++) {
        WnckWorkspace *wws = wmOperator->workspaceQList->at(i);
        UkwsWorkspaceBox *wsbox = new UkwsWorkspaceBox;
        UkwsIndicator *ind = new UkwsIndicator;

        wsbox->index = i;
        wsbox->setTitle(wnck_workspace_get_name(wws));
        wsbox->setSizeByThumbnailSize(w, h);
        wsbox->setWnckWorkspace(wws);
        wsbox->setBackground(wsboxBackground);

        ind->setFixedWidth(screenRect.width() * config->workspacePrimaryAreaUnits
                           / config->workspaceAllUnits);
        ind->setConfig(config);
        ind->wmOperator->screen = screen;
        ind->wmOperator->workspace = wws;
        ind->wmOperator->needCheckWorkspace = true;
        ind->wmOperator->needCheckScreen = false;
        ind->index = i;
//        ind->reShow(UkwsIndicator::ShowModeTiling, minScale);
        ind->setAcceptDrops(true);
        ind->setObjectName(UKWS_OBJ_IND_MAINWIDGET_Tiling);

        connect(wsbox, &UkwsWorkspaceBox::doHover,
                this, &UkwsWorkspaceManager::setShowingIndicator);
        connect(wsbox, &UkwsWorkspaceBox::selectedWorkspace,
                this, &UkwsWorkspaceManager::changeWorkspace);
        connect(wsbox, &UkwsWorkspaceBox::windowChangeWorkspace,
                this, &UkwsWorkspaceManager::moveWindowWorkspace);
        connect(ind, &UkwsIndicator::isSelected,
                this, &UkwsWorkspaceManager::selectWinbox);
        connect(ind, &UkwsIndicator::windowViewPixmapChange,
                this, &UkwsWorkspaceManager::doIndicatorWindowViewChange);

        spaceBoxList.append(wsbox);
        indList.append(ind);

        wsboxLayout->addWidget(wsbox);
        indStack->addWidget(ind);
    }

    for (int i = 0; i < size; i++)
        indList.at(i)->reShow(UkwsIndicator::ShowModeTiling, minScale);

    // 显示当前的工作区
    WnckWorkspace *workspace = wnck_screen_get_active_workspace(screen);
    int workspaceNum = wnck_workspace_get_number(workspace);
    setShowingIndicator(workspaceNum);

    wsboxLayout->addWidget(newWorkspaceBtn);
    newWorkspaceBtn->setSizeByButtonSize(w, h);
//    newWorkspaceBtn->show();
    newWorkspaceBtn->hide();

    if (size >= UKWS_WORKSPACE_MAX)
        newWorkspaceBtn->hide();
}

void UkwsWorkspaceManager::reShow(int minScale)
{
    if (showStatus != UkwsWidgetShowStatus::Hidden) {
        return;
    }

    showStatus = UkwsWidgetShowStatus::Constructing;

    // 先让界面显示出来，从而让Qt完成布局重绘
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    this->move(primaryScreen->geometry().topLeft());
    this->showFullScreen();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

    reloadWorkspace(minScale);

    this->activateWindow();

    showStatus = UkwsWidgetShowStatus::Shown;
}

void UkwsWorkspaceManager::reHide()
{
    if (showStatus != UkwsWidgetShowStatus::Shown) {
        return;
    }

    showStatus = UkwsWidgetShowStatus::Destructing;

    this->hide();
    // 优先处理hide事件，完成后再清理后续
    QCoreApplication::processEvents();

    cleanAllWorkspace();

    showStatus = UkwsWidgetShowStatus::Hidden;
    emit isHidden();
}

void UkwsWorkspaceManager::setShowingIndicator(int index)
{
    int size = indList.size();
    int idx = index;

    if (size <= 0) {
        qCritical("Indicator list is empty, but get index:%d ...\n", index);
        return;
    }

    // wnck在部分窗管上会出现各种异常问题，所以需要在此做边界判断
    if ((idx < 0) || (idx >= indList.size())) {
        qCritical("Illegal index values: %d, set index to 0\n", index);
        idx = 0;
    }

    for (int i = 0; i < size; i++)
        if (i != idx)
            indList.at(i)->hide();
        else
            indList.at(i)->show();

    indStack->setCurrentIndex(idx);
    UkwsIndicator *ind = indList.at(idx);
    ind->flowReLayout();

    UkwsWorkspaceBox *wsbox;
    foreach (wsbox, spaceBoxList) {
        wsbox->setSelectStatus(false);
    }
    wsbox = spaceBoxList.at(idx);
    wsbox->setSelectStatus(true);
}

void UkwsWorkspaceManager::selectWinbox(bool needActivate)
{
    if (needActivate) {
        UkwsIndicator *ind = static_cast<UkwsIndicator *>(indStack->currentWidget());
        ind->acitveSelectedWindow();
        reHide();
    }
}

void UkwsWorkspaceManager::changeWorkspace(int index)
{
    WnckWorkspace *workspace = wnck_screen_get_workspace(wmOperator->screen, index);
    unsigned long timestamp = QX11Info::getTimestamp();
    wnck_workspace_activate(workspace, timestamp);
    reHide();
}

void UkwsWorkspaceManager::moveWindowWorkspace(int wbIndex, int srcWsIndex, int dstWsIndex)
{
    if (srcWsIndex >= indList.size() || (dstWsIndex >= indList.size()))
        return;

    UkwsIndicator *srcInd = indList.at(srcWsIndex);
    UkwsIndicator *dstInd = indList.at(dstWsIndex);
    UkwsWindowBox *wb = srcInd->getWinbox(wbIndex);

    if (wb == nullptr)
        return;

    wb->moveToWorkspace(dstWsIndex);

//    wb->setWinboxSizeByHeight(boxMinHeight);

    // 从源indicator中移除
    srcInd->rmWinbox(wb);

    // 根据dst的布局，更新winbox的大小

    // 添加到目标indicator中
    dstInd->addWinbox(wb);

    // 更新桌面视图
    srcInd->updateWindowViewPixmap(true);
    dstInd->updateWindowViewPixmap(true);

    // 重新布局
//    srcInd->flowReLayout();
//    dstInd->flowReLayout();
}

void UkwsWorkspaceManager::doIndicatorWindowViewChange(int indIndex)
{
    if (indIndex >= indList.size())
        return;

    if (indIndex >= spaceBoxList.size())
        return;

    UkwsWorkspaceBox *wsbox = spaceBoxList.at(indIndex);
    wsbox->updateDesktopViewThumbnail(indList.at(indIndex)->getWindowView());
}

QString UkwsWorkspaceManager::getBackgroundFileByGSettings(QString schemaDir,
                                                           QString schemaUrl,
                                                           QString keyName)
{
    QUrl url;
    QString fileUrl = QString("");

    /*
     * 不再使用glib的方式获取gsettings，使用QGsettings
     */
#if 0
    GSettingsSchemaSource *schema_source = NULL;
    GSettingsSchema *schema = NULL;
    GSettings *settings = NULL;
    GVariant *value = NULL;
    const gchar *name;

    schema_source = g_settings_schema_source_new_from_directory (schemaDir.toStdString().c_str(),
                                                                 g_settings_schema_source_get_default(),
                                                                 TRUE, NULL);
    if (!schema_source) {
        qWarning() << "Cannot get gsettings schema form" << schemaDir;
        goto GSFREE;
    }

    schema = g_settings_schema_source_lookup(schema_source,
                                             schemaUrl.toStdString().c_str(),
                                             FALSE);
    if (!schema) {
        qWarning() << "Cannot get gsettings schema source:" << schemaUrl;
        goto GSFREE;
    }

    settings = g_settings_new_full(schema, NULL, NULL);
    if (!settings) {
        qWarning() << "Cannot get gsettings from schema";
        goto GSFREE;
    }

    value = g_settings_get_value(settings, keyName.toStdString().c_str());
    if (!value)  {
        qWarning() << "Cannot get key:" << keyName;
        goto GSFREE;
    }

    name = g_variant_get_string(value, NULL);
    if (!name)  {
        qWarning() << QString("Cannot get gsettings key(%1) value").arg(keyName);
        goto GSFREE;
    }

    url = QUrl::fromEncoded(QByteArray(name));
    fileUrl = url.toString().replace(QRegExp("^file:/"), "");

GSFREE:
    if (value != NULL)
        g_variant_unref(value);

    if (settings != NULL)
        g_object_unref(settings);

    if (schema != NULL)
        g_settings_schema_unref(schema);

    if (schema_source != NULL)
        g_settings_schema_source_unref(schema_source);
#endif

    Q_UNUSED(schemaDir);

    if (QGSettings::isSchemaInstalled(schemaUrl.toUtf8())) {
        QGSettings gsettings(schemaUrl.toUtf8());

        const QStringList keyList = gsettings.keys();
//        qDebug() << schemaUrl << "Key List:" << keyList;
        if (keyList.contains(keyName)) {
            QString name = gsettings.get(keyName).toString();
            url = QUrl::fromLocalFile(name);
            fileUrl = url.toString().replace(QRegExp("^file:/"), "");
//            qDebug() << "Get background file:" << fileUrl;
        } else {
            qWarning() << QString("Cannot get gsettings key(%1) value").arg(keyName);
        }
    }

    return fileUrl;
}

QString UkwsWorkspaceManager::getBackgroundColorGSettings(QString schemaDir,
                                                          QString schemaUrl,
                                                          QString keyName)
{
    QString colorString = "";

    /*
     * 不再使用glib的方式获取gsettings，使用QGsettings
     */

    Q_UNUSED(schemaDir);

    if (QGSettings::isSchemaInstalled(schemaUrl.toUtf8())) {
        QGSettings gsettings(schemaUrl.toUtf8());

        const QStringList keyList = gsettings.keys();
        if (keyList.contains(keyName)) {
            colorString = gsettings.get(keyName).toString();
//            qDebug() << "Get background color:" << colorString;
        } else {
            qWarning() << QString("Cannot get gsettings key(%1) value").arg(keyName);
        }
    }

    return colorString;
}

void UkwsWorkspaceManager::getBackground()
{
    QString filePath;

    filePath = getBackgroundFileByGSettings("/usr/share/glib-2.0/schemas/",
                                            "org.mate.background",
                                            "pictureFilename");

    if (filePath == "") {
        // 无法从ukui桌面环境获取背景文件，则获取背景颜色
        QString colorName = getBackgroundColorGSettings("/usr/share/glib-2.0/schemas/",
                                                        "org.mate.background",
                                                        "primaryColor");
        if (colorName != "") {
            if (background.isNull()) {
                background = QPixmap(480, 270);
            }
            background.fill(QColor(colorName));
            return;
        }

        // 无法获取背景色，则继续获取gnome桌面环境的背景图片
        filePath = getBackgroundFileByGSettings("/usr/share/glib-2.0/schemas/",
                                                "org.gnome.desktop.background",
                                                "pictureUri");
        if (filePath == "") {
            qWarning() << "Cannot get background image, use default:" << UKWS_WORKSPACE_DEFAULT_BACKGROUND;
            filePath = UKWS_WORKSPACE_DEFAULT_BACKGROUND;
        }
    }

    QImage img;
    img.load(filePath);
    background = QPixmap::fromImage(img);
}

void UkwsWorkspaceManager::setBackgroundImage(int width, int height)
{
    // 计算主区域宽度
    int w, h, x;

    if (width == 0)
        w = size().width();
    else
        w = width;

    x = w * config->workspacePrimaryAreaUnits / config->workspaceAllUnits;

    if (height == 0)
        h = size().height();
    else
        h = height;


    // 背景图置灰
    QPixmap tempPixmap = background.scaled(size(), Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation);
    UkwsStackBlur blurrer;
    blurrer.setOrigImage(tempPixmap.toImage());
    blurrer.setReduce(2);
    blurrer.setRadius(16);
    blurrer.setBlurRect(QRect(x, 0, w - x + 1, size().height()));
    blurrer.calculate();
    tempPixmap = QPixmap::fromImage(blurrer.blurImage());

    QPainter painter;
    painter.begin(&tempPixmap);

    // 设置左半区域遮罩
    painter.fillRect(0, 0, x, h, QColor(19, 19, 20, 127));

    // 设置右半区域遮罩
    painter.fillRect(x, 0, w - x, h, QColor(19, 19, 20, 178));
    painter.end();

    setAutoFillBackground(true);   // 这个属性一定要设置
    QPalette pal(palette());
    pal.setBrush(QPalette::Window,
                     QBrush(tempPixmap));
    setPalette(pal);
}

void UkwsWorkspaceManager::cleanAllWorkspace()
{
    // 清理workspace box
    foreach(UkwsWorkspaceBox *wsbox, spaceBoxList) {
        wsboxLayout->removeWidget(wsbox);
        spaceBoxList.removeOne(wsbox);
        wsbox->deleteLater();
    }
    spaceBoxList.clear();
    wsboxLayout->removeWidget(newWorkspaceBtn);

    // 清理indicator
    foreach(UkwsIndicator *ind, indList) {
        indStack->removeWidget(ind);
        indList.removeOne(ind);
        ind->cleanAllWinbox();
        ind->deleteLater();
    }
    indList.clear();
}

void UkwsWorkspaceManager::setConfig(UkwsConfig *config)
{
    this->config = config;
}

bool UkwsWorkspaceManager::eventFilter(QObject *object, QEvent *event)
{
    if (object == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            changeWorkspace(indStack->currentIndex());
            reHide();
            return true;
        }

        if (event->type() == QEvent::MouseMove) {
            return true;
        }

        // QEvent::KeyPress == 6
        if (event->type() == 6) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                reHide();
                return true;
            }
        }

        if (event->type() == QEvent::WindowDeactivate) {
            if (showStatus == UkwsWidgetShowStatus::Shown ||
                     showStatus == UkwsWidgetShowStatus::Constructing) {
                changeWorkspace(indStack->currentIndex());
                reHide();
            }
            return true;
        }
    }

    return false;
}
