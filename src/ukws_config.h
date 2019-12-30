#ifndef UKWS_CONFIG_H
#define UKWS_CONFIG_H

#include <QObject>
#include <QString>

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
    int logLevel;
    float indicatorMinRatio;
    QString theme;

private:
    QString configFilePath;
};

#endif // UKWS_CONFIG_H
