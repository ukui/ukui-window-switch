QT  += core gui x11extras dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

TARGET = ukui-window-switch
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/ukws_indicator.cpp \
    src/ukws_stack_blur.cpp \
    src/ukws_wnck_operator.cpp \
    src/ukws_helper.cpp \
    src/ukws_window_box.cpp \
    src/ukws_window_extra_label.cpp \
    src/ukws_flowlayout.cpp \
    src/ukws_config.cpp \
    src/ukws_dbus.cpp \
    src/ukws_workspace_manager.cpp \
    src/ukws_workspace_box.cpp \
    src/qhotkey.cpp \
    src/qhotkey_x11.cpp \
    src/ukws_manager.cpp \
    src/ukws_worker.cpp \
    src/ukws_fake_desktop.cpp \
    src/ukws_window_info.cpp \
    src/ukws_workspace_multiview.cpp

HEADERS += \
    src/ukws_indicator.h \
    src/ukws_stack_blur.h \
    src/ukws_wnck_operator.h \
    src/ukws_helper.h \
    src/ukws_window_box.h \
    src/ukws_window_extra_label.h \
    src/ukws_flowlayout.h \
    src/ukws_config.h \
    src/ukws_common.h \
    src/ukws_dbus.h \
    src/ukws_workspace_manager.h \
    src/ukws_workspace_box.h \
    src/qhotkey_p.h \
    src/qhotkey.h \
    src/ukws_manager.h \
    src/ukws_worker.h \
    src/ukws_fake_desktop.h \
    src/ukws_window_info.h \
    src/ukws_workspace_multiview.h

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += x11 libwnck-3.0 gdk-3.0

DISTFILES += \
    data/default-background.jpg \
    data/org.ukui.WindowSwitch.service \
    data/ukui-window-switch.conf \
    theme/default.qss

target.path = /usr/bin/

services.path = /usr/share/dbus-1/services/
services.files = data/org.ukui.WindowSwitch.service

theme.path = /usr/share/ukui-window-switch/theme/
theme.files = theme/default.qss

conf.path = /etc/ukui/ukui-window-switch/
conf.files = data/ukui-window-switch.conf

background.path = /usr/share/ukui-window-switch/data/
background.files = data/default-background.jpg

INSTALLS += target services theme conf background
