#include "ukws_workspace_multiview.h"

#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QStyleOption>
#include <QPainter>
#include <QX11Info>
#include <math.h>

UkwsWorkspaceMultiview::UkwsWorkspaceMultiview(QWidget *parent) : QWidget(parent)
{
    wmOperator = new UkwsWnckOperator;
    showStatus = UkwsWidgetShowStatus::Hidden;
    config = nullptr;

    mainLayout = new QGridLayout();
    mainLayout->setMargin(0);

    this->setLayout(mainLayout);
    this->resize(1200, 600);

    this->installEventFilter(this);
}

void UkwsWorkspaceMultiview::reloadWorkspace(int lineMax)
{
    cleanAllWorkspace();

    QDesktopWidget *desktop = QApplication::desktop();
    int screenNum = desktop->screenNumber(this);
//    QRect screenRect = desktop->screenGeometry(screenNum);
    QRect screenRect = QGuiApplication::screens().at(screenNum)->geometry();
    WnckScreen *screen = wnck_screen_get(screenNum);

    // 设置视图大小
    this->resize(screenRect.width(), screenRect.height());

    // 获取工作区列表
    wmOperator->updateWorkspaceList();
    int size = wmOperator->workspaceQList->size();

    // 获取背景图片
    this->getBackground();

    // 设置工作区视图的最底层背景
    this->setBackgroundImage();

    // 计算工作区布局的行数与列数
    int colMax = (int)ceil(sqrt(size));
    if (colMax > lineMax)
        colMax = lineMax;

    for (int i = 0; i < size; i++) {
        WnckWorkspace *wws = wmOperator->workspaceQList->at(i);
        UkwsFakeDesktop *fd = new UkwsFakeDesktop;
        int row = i / colMax;
        int col = i % colMax;

        fd->index = i;
//        fd->setconfig(config);
        fd->wmOperator->screen = screen;
        fd->wmOperator->workspace = wws;
        fd->wmOperator->needCheckWorkspace = true;
        fd->wmOperator->needCheckScreen = false;
        fd->setAcceptDrops(true);
//        fd->setObjectName(UKWS_OBJ_IND_MAINWIDGET_Tiling);
        fakeDesktopList.append(fd);

        mainLayout->addWidget(fd, row, col);
    }

    for (int i = 0; i < size; i++)
        fakeDesktopList.at(i)->reShow();
}

void UkwsWorkspaceMultiview::reShow(int lineMax)
{
    if (showStatus != UkwsWidgetShowStatus::Hidden) {
        return;
    }

    showStatus = UkwsWidgetShowStatus::Constructing;

    reloadWorkspace(lineMax);
    this->show();
    this->activateWindow();

    showStatus = UkwsWidgetShowStatus::Shown;
}

QString UkwsWorkspaceMultiview::getBackgroundFileByGSettings(QString schemaDir,
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

void UkwsWorkspaceMultiview::getBackground()
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

void UkwsWorkspaceMultiview::setBackgroundImage()
{
    setAutoFillBackground(true);   // 这个属性一定要设置
    QPalette painter(palette());
    painter.setBrush(QPalette::Window,
                     QBrush(background.scaled(size(), Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation)));
    setPalette(painter);
}

void UkwsWorkspaceMultiview::cleanAllWorkspace()
{

}

void UkwsWorkspaceMultiview::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

    QWidget::paintEvent(event);
}

bool UkwsWorkspaceMultiview::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    Q_UNUSED(event);

    return false;
}
