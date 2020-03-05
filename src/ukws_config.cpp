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

#include "ukws_config.h"

#include <QDir>
#include <QSettings>
#include <QDebug>

#ifndef UKWS_DATA_DEFAULT_DIR
#define UKWS_DATA_DEFAULT_DIR "/usr/share/ukui-window-switch/"
#endif

QString defaultTheme="QWidget#indicator_sub_widget {\
        background-color: rgba(0, 0, 0, 0);\
        border-style: none;\
        border-width: 0px;\
        padding: 0px;\
    }\
\
    QWidget#indicator_main_widget {\
        background-color: rgba(0, 0, 0, 176);\
        border-style: none;\
        border-width: 0px;\
        padding: 0px;\
    }\
\
    QWidget#ws_manager_sub_widget {\
        border-radius: 1px;\
        border-style: none;\
        border-width: 0px;\
        padding: 0px;\
    }\
\
    QWidget#ws_manager_main_widget {\
        border-style: none;\
        border-width: 0px;\
        padding: 0px;\
    }\
\
    UkwsWindowExtraLabel#winbox-wintitle {\
        font-size: 14px;\
        color: white;\
        padding-left: 3px;\
    }\
\
    UkwsWindowExtraLabel#winbox-thumbnail:hover {\
        border: 2px solid rgb(255, 255, 255);\
    }\
\
    UkwsWindowExtraLabel#winbox-thumbnail:!hover {\
        border: 2px solid rgba(255, 255, 255, 0);\
    }\
\
    QWidget#winbox:hover {\
        padding: 0px;\
        margin: 0px;\
        border: 2px solid rgba(255, 255, 255, 255);\
    }\
\
    QWidget#winbox:!hover {\
        padding: 0px;\
        margin: 0px;\
        border: 2px solid rgba(255, 255, 255, 0);\
    }";

UkwsConfig::UkwsConfig()
{

}

void UkwsConfig::configReload()
{
    QSettings conf(configFilePath, QSettings::IniFormat);
    bool ok;

    // 获取缩略图质量
    QString mode;
    mode = conf.value("ScaledMode").toString();
    if (mode == "fast") {
        scaledMode = Qt::FastTransformation;
    } else if (mode == "smooth") {
        scaledMode = Qt::SmoothTransformation;
    } else {
        scaledMode = Qt::FastTransformation;
        qWarning("ScaledMode parameter type error, use defalut mode: fast");
    }

    // 获取窗口装饰区shadow的宽度
    frameShadowWidth = conf.value("FrameShadowWidth").toInt(&ok);
    if (!ok) {
        frameShadowWidth = 0;
        qWarning("FrameShadowWidth parameter type error, use defalut level: %d", 0);
    }

    // 获取窗口装饰区shadow的顶部偏移
    frameshadowTopOffset = conf.value("FrameshadowTopOffset").toInt(&ok);
    if (!ok) {
        frameshadowTopOffset = 0;
        qWarning("FrameshadowTopOffset parameter type error, use defalut level: %d", 0);
    }

    // 获取打印的日志等级
    logLevel = conf.value("LogLevel").toInt(&ok);
    if (!ok) {
        logLevel = DEFAULT_LOG_LEVEL;
        qWarning("LogLevel parameter type error, use defalut level: %d", DEFAULT_LOG_LEVEL);
    }

    // 获取主题名
    this->themeName = conf.value("Theme").toString();
    if (this->themeName == "") {
        this->themeName = "default";
        qWarning("Theme parameter not set, use defalut");
    }

    // 获取工作区视图主区域宽度的unit值
    workspacePrimaryAreaUnits = conf.value("WorkspacePrimaryAreaUnits").toInt(&ok);
    if (!ok) {
        workspacePrimaryAreaUnits = UKWS_WS_PRIMARY_AREA_UNITS;
        qWarning("WorkspacePrimaryAreaUnits parameter type error, use defalut"
                 " value: %d", workspacePrimaryAreaUnits);
    }

    // 获取工作区视图侧边栏宽度的unit值
    workspaceSidebarUnits = conf.value("WorkspaceSidebarUnits").toInt(&ok);
    if (!ok) {
        workspaceSidebarUnits = UKWS_WS_SIDEBAR_UNITS;
        qWarning("WorkspaceSidebarUnits parameter type error, use defalut"
                 " value: %d", workspaceSidebarUnits);
    }

    // 获取工作区视图item宽度的unit值
    workspaceItemUnits = conf.value("WorkspaceItemUnits").toInt(&ok);
    if (!ok) {
        workspaceItemUnits = UKWS_WS_ITEM_UNITS;
        qWarning("WorkspaceItemUnits parameter type error, use defalut"
                 " value: %d", workspaceItemUnits);
    }

    workspaceAllUnits = workspacePrimaryAreaUnits + workspaceSidebarUnits;

    // 获取主题内容
    QString ukwsThemeString;
    QFile ukwsTheme;
    QFileInfo themeFileInfo;
    QStringList ukswDirList;
    ukswDirList << "/usr/share/ukui-window-switch/" <<
                   "/home/droiing/workspace/ukui-window-switch/ukui-window-switch/" <<
                   UKWS_DATA_DEFAULT_DIR;
    foreach(QString ukwsDir, ukswDirList) {
        themeFileInfo.setFile(ukwsDir + "/theme/" + this->themeName + ".qss");
        qDebug() << "Theme file check:" << themeFileInfo.absoluteFilePath();
        if (themeFileInfo.exists())
            ukwsTheme.setFileName(themeFileInfo.absoluteFilePath());
    }
    if (ukwsTheme.fileName() == "") {
        ukwsThemeString = defaultTheme;
        qDebug() << "Loading internal default theme";
    } else {
        qDebug() << "Loading theme file:" << ukwsTheme.fileName();
        ukwsTheme.open(QFile::ReadOnly);
        ukwsThemeString = ukwsTheme.readAll();
        ukwsTheme.close();
    }
    this->themeString = ukwsThemeString;
}


QString UkwsConfig::configFile()
{
    return configFilePath;
}

void UkwsConfig::setConfigFile(QString configFilePath)
{
    QDir dir(configFilePath);
    this->configFilePath = dir.absolutePath();
}

void UkwsConfig::setConfigFile(QString configFileName, QString configDirPath)
{
    QDir dir(configDirPath);
    this->configFilePath = dir.absoluteFilePath(configFileName);
}
