INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/qxtglobal.h \
		   $$PWD/qxtglobalshortcut.h \
		   $$PWD/qxtglobalshortcut_p.h

SOURCES += $$PWD/qxtglobalshortcut.cpp

unix:SOURCES += $$PWD/qxtglobalshortcut_x11.cpp

QT += gui-private
