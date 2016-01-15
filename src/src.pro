lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

QT += core gui
QT += widgets
QT += xml

TARGET = cangaroo
TEMPLATE = app
CONFIG += warn_on
CONFIG += link_pkgconfig

PKGCONFIG += libnl-3.0
PKGCONFIG += libnl-route-3.0

DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac


SOURCES += main.cpp\
        mainwindow.cpp \
    Backend.cpp \
    MdiWindow.cpp

HEADERS  += mainwindow.h \
    Backend.h \
    MdiWindow.h

FORMS    += mainwindow.ui

RESOURCES = cangaroo.qrc

include($$PWD/model/model.pri)
include($$PWD/driver/driver.pri)
include($$PWD/parser/dbc/dbc.pri)
include($$PWD/window/TraceWindow/TraceWindow.pri)
include($$PWD/window/SetupDialog/SetupDialog.pri)
include($$PWD/window/LogWindow/LogWindow.pri)
include($$PWD/window/GraphWindow/GraphWindow.pri)
