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
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <QStyleOption>
#include <QPainter>
#include <QX11Info>

// ukws_helper.h包含X.h，必须放到所有Qt头文件之前
#include "ukws_helper.h"

UkwsWorkspaceManager::UkwsWorkspaceManager(QWidget *parent) : QWidget(parent)
{
    wmOperator = new UkwsWnckOperator;
    indStack = new QStackedWidget;
    wsboxLayout = new QVBoxLayout;
    showStatus = UkwsWidgetShowStatus::Hidden;

    mainLayout = new QHBoxLayout();
    mainLayout->addWidget(indStack);
    mainLayout->addLayout(wsboxLayout);
    mainLayout->setAlignment(Qt::AlignHCenter);
    wsboxLayout->setAlignment(Qt::AlignTop);

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

    QDesktopWidget *desktop = QApplication::desktop();
    int screenNum = desktop->screenNumber(this);
//    QRect screenRect = desktop->screenGeometry(screenNum);
    QRect screenRect = QGuiApplication::screens().at(screenNum)->geometry();
    WnckScreen *screen = wnck_screen_get(screenNum);
    float scale = 1.0 / 6;
    int w = screenRect.width() * scale;
    int h = screenRect.height() * scale;

    this->resize(screenRect.width(), screenRect.height());

    wmOperator->updateWorkspaceList();
    int size = wmOperator->workspaceQList->size();

    this->getBackground();
    this->setBackgroundImage();

    QPixmap wsboxBackground = background.scaled(QSize(w, h),
                                                Qt::IgnoreAspectRatio,
                                                Qt::SmoothTransformation);

    for (int i = 0; i < size; i++) {
        WnckWorkspace *wws = wmOperator->workspaceQList->at(i);
        UkwsWorkspaceBox *wsbox = new UkwsWorkspaceBox;
        UkwsIndicator *ind = new UkwsIndicator;

        wsbox->index = i;
        wsbox->setTitle(wnck_workspace_get_name(wws));
        wsbox->setFixedSize(w, h);
        wsbox->setWnckWorkspace(wws);
        wsbox->setThumbnail(wsboxBackground);

        ind->setFixedWidth(screenRect.width() - w - 10);
        ind->wmOperator->screen = screen;
        ind->wmOperator->workspace = wws;
        ind->wmOperator->needCheckWorkspace = true;
        ind->wmOperator->needCheckScreen = false;
        ind->index = i;
        ind->reShow(UkwsIndicator::ShowModeTiling, minScale);
        ind->setAcceptDrops(true);

        connect(wsbox, &UkwsWorkspaceBox::doHover,
                this, &UkwsWorkspaceManager::setShowingIndicator);
        connect(wsbox, &UkwsWorkspaceBox::selectedWorkspace,
                this, &UkwsWorkspaceManager::changeWorkspace);
        connect(wsbox, &UkwsWorkspaceBox::windowChangeWorkspace,
                this, &UkwsWorkspaceManager::moveWindowWorkspace);
        connect(ind, &UkwsIndicator::isSelected,
                this, &UkwsWorkspaceManager::selectWinbox);

        spaceBoxList.append(wsbox);
        indList.append(ind);

        wsboxLayout->addWidget(wsbox);
        indStack->addWidget(ind);
    }
}

void UkwsWorkspaceManager::reShow(int minScale)
{
    if (showStatus != UkwsWidgetShowStatus::Hidden) {
        return;
    }

    showStatus = UkwsWidgetShowStatus::Constructing;

    reloadWorkspace(minScale);
    this->show();
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
}

void UkwsWorkspaceManager::setShowingIndicator(int index)
{
    int size = indList.size();
    for (int i = 0; i < size; i++)
        if (i != index)
            indList.at(i)->hide();
        else
            indList.at(i)->show();

    indStack->setCurrentIndex(index);
    UkwsIndicator *ind = indList.at(index);
    ind->flowReLayout();

    UkwsWorkspaceBox *wsbox;
    foreach (wsbox, spaceBoxList) {
        wsbox->setBoxStyle("QWidget {font-size: 14px; color: Silver; padding-left: 0px;}"
                           "QWidget#wsbox-thumbnail {"
                           "padding: 0px; margin: 0px;"
                           "border: 2px solid rgba(255, 255, 255, 0);"
                           "}");
    }
    wsbox = spaceBoxList.at(index);
    wsbox->setBoxStyle("QWidget {font-size: 14px; color: White; padding-left: 0px;}"
                       "QWidget#wsbox-thumbnail {"
                       "padding: 0px; margin: 0px;"
                       "border: 2px solid rgba(255, 255, 255, 255);"
                       "}");
}

void UkwsWorkspaceManager::selectWinbox(bool needActivate)
{
    if (needActivate) {
        UkwsIndicator *ind = static_cast<UkwsIndicator *>(indStack->currentWidget());
        ind->acitveSelectedWindow();
        reHide();
        emit isHidden();
    }
}

void UkwsWorkspaceManager::changeWorkspace(int index)
{
    WnckWorkspace *workspace = wnck_screen_get_workspace(wmOperator->screen, index);
    unsigned long timestamp = QX11Info::getTimestamp();
    wnck_workspace_activate(workspace, timestamp);
    reHide();
    emit isHidden();
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

    // 重新布局
//    srcInd->flowReLayout();
//    dstInd->flowReLayout();
}

QString UkwsWorkspaceManager::getBackgroundFileByGSettings(QString schemaDir,
                                                    QString schemaUrl,
                                                    QString keyName)
{
    QUrl url;
    QString fileUrl = QString("");

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

    return fileUrl;
}

void UkwsWorkspaceManager::getBackground()
{
    QString filePath;

    filePath = getBackgroundFileByGSettings("/usr/share/glib-2.0/schemas/",
                                            "org.mate.background",
                                            "picture-filename");

    if (filePath == "") {
        filePath = getBackgroundFileByGSettings("/usr/share/glib-2.0/schemas/",
                                                "org.gnome.desktop.background",
                                                "picture-uri");
    }

    if (filePath == "") {
        qWarning() << "Cannot get background image, use default:" << UKWS_WORKSPACE_DEFAULT_BACKGROUND;
        filePath = UKWS_WORKSPACE_DEFAULT_BACKGROUND;
    }

    QImage img;
    img.load(filePath);
    background = QPixmap::fromImage(img);

}

void UkwsWorkspaceManager::setBackgroundImage()
{
    setAutoFillBackground(true);   // 这个属性一定要设置
    QPalette painter(palette());
    painter.setBrush(QPalette::Window,
                     QBrush(background.scaled(size(), Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation)));
    setPalette(painter);
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

    // 清理indicator
    foreach(UkwsIndicator *ind, indList) {
        indStack->removeWidget(ind);
        indList.removeOne(ind);
        ind->cleanAllWinbox();
        ind->deleteLater();
    }
    indList.clear();
}

bool UkwsWorkspaceManager::eventFilter(QObject *object, QEvent *event)
{
    if (object == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
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
                emit isHidden();

                return true;
            }
        }

        if (event->type() == QEvent::WindowDeactivate) {
            if (showStatus == UkwsWidgetShowStatus::Shown ||
                     showStatus == UkwsWidgetShowStatus::Constructing) {
                reHide();
                emit isHidden();
            }

            return true;
        }
    }

    return false;
}
