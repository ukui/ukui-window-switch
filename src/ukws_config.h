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

#ifndef UKWS_CONFIG_H
#define UKWS_CONFIG_H

#include <QObject>
#include <QString>

#ifndef UKWS_CONF_DEFAULT_DIR
#define UKWS_CONF_DEFAULT_DIR "/etc/ukui/ukui-window-switch/"
#endif

#define DEFAULT_LOG_LEVEL   5

class UkwsConfig : public QObject
{
    Q_OBJECT
public:
    explicit UkwsConfig();

    void configReload();

    QString configFile();
    void setConfigFile(QString configFilePath);
    void setConfigFile(QString configFileName, QString configDirPath);


    Qt::TransformationMode scaledMode;
    int frameShadowWidth;
    int frameshadowTopOffset;
    int logLevel;
    QString themeName;
    QString themeString;

private:
    QString configFilePath;
};

#endif // UKWS_CONFIG_H
