#-------------------------------------------------
#
# Project created by QtCreator 2015-12-20T20:40:42
#
#-------------------------------------------------

# This project requires at least v5.x to compile
lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

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
include($$PWD/parser/dbc/dbc.pri)
include($$PWD/window/TraceWindow/TraceWindow.pri)
include($$PWD/window/SetupDialog/SetupDialog.pri)
include($$PWD/window/LogWindow/LogWindow.pri)
include($$PWD/window/GraphWindow/GraphWindow.pri)
