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
