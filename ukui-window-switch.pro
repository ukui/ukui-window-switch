#-------------------------------------------------
#
# Project created by QtCreator 2017-07-24T20:25:09
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-window-switch
TEMPLATE = app

DESC_XML = data/org.ukui.ukwm.UkwmPlugin.xml
GENERATED_CH = ukui_plugin_generated
system(gdbus-codegen --interface-prefix org.ukui.ukwm --generate-c-code $${GENERATED_CH} $${DESC_XML})

SOURCES += main.cpp \
        mainwindow.cpp \
        alt_tab_list.c  \
        ukui_plugin_generated.c \
        mylabel.cpp \
        keyeventmonitor.cpp

HEADERS += mainwindow.h alt_tab_list.h ukui_plugin_generated.h \
        mylabel.h \
        keyeventmonitor.h

FORMS += mainwindow.ui

include(qxtglobalshortcut/qxtglobalshortcut.pri)

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gio-unix-2.0 gobject-2.0 gmodule-2.0 glib-2.0 xcb xcb-util x11 xext xtst

DISTFILES += data/org.ukui.ukwm.UkwmPlugin.xml
