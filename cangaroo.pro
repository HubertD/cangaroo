#-------------------------------------------------
#
# Project created by QtCreator 2015-12-20T20:40:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cangaroo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    Logger.cpp \
    Backend.cpp

HEADERS  += mainwindow.h \
    Logger.h \
    Backend.h

FORMS    += mainwindow.ui

RESOURCES = cangaroo.qrc

include($$PWD/model/model.pri)
include($$PWD/driver/driver.pri)
include($$PWD/views/views.pri)
include($$PWD/parser/dbc/dbc.pri)
include($$PWD/window/TraceWindow/TraceWindow.pri)
include($$PWD/window/SetupDialog/SetupDialog.pri)
include($$PWD/window/LogWindow/LogWindow.pri)
