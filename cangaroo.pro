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
    Logger.cpp

HEADERS  += mainwindow.h \
    Logger.h

FORMS    += mainwindow.ui

RESOURCES = cangaroo.qrc

include($$PWD/model/model.pri)
include($$PWD/drivers/drivers.pri)
include($$PWD/views/views.pri)
include($$PWD/parser/dbc/dbc.pri)
include($$PWD/window/TraceWindow/TraceWindow.pri)
