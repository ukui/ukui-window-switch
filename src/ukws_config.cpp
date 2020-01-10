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

UkwsConfig::UkwsConfig()
{

}

void UkwsConfig::configReload()
{
    QSettings conf(configFilePath, QSettings::IniFormat);
    bool ok;

    QString mode;
    mode = conf.value("ScaledMode").toString();
    if (mode == "fast") {
        scaledMode = Qt::FastTransformation;
    } else if (mode == "smooth") {
        scaledMode = Qt::SmoothTransformation;
    } else {
        scaledMode = Qt::FastTransformation;
        qWarning("scaled_mode parameter type error, use defalut mode: fast");
    }

    this->theme = conf.value("Theme").toString();
    if (this->theme == "") {
        this->theme = "default";
        qWarning("theme parameter not set, use defalut");
    }


    logLevel = conf.value("LogLevel").toInt(&ok);
    if (!ok) {
        logLevel = DEFAULT_LOG_LEVEL;
        qWarning("log_level parameter type error, use defalut level: %d", DEFAULT_LOG_LEVEL);
    }
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
