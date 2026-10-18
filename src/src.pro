lessThan(QT_MAJOR_VERSION, 6): error("requires Qt 6")

QT += core gui
QT += widgets
QT += xml
QT += charts
QT += serialport

TARGET = cangaroo
TEMPLATE = app

# Read version from VERSION file
CANGAROO_VERSION_RAW = $$cat($$PWD/../VERSION)
CANGAROO_VERSION_STR_QUOTED = \"$$CANGAROO_VERSION_RAW\"

QMAKE_SUBSTITUTES += version.h.in
INCLUDEPATH += .
CONFIG += warn_on
CONFIG += link_pkgconfig

TRANSLATIONS = \
    translations/cangaroo_de_DE.ts \
    translations/i18n_en_us.ts \
    translations/i18n_zh_cn.ts
RC_ICONS = cangaroo.ico

DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac


SOURCES += main.cpp\
    mainwindow.cpp \
    core/CanLogParser.cpp \
    core/CanReplayer.cpp \
    window/ConditionalLoggingDialog.cpp

HEADERS  += mainwindow.h \
    core/CanLogParser.h \
    core/CanReplayer.h \
    window/ConditionalLoggingDialog.h

FORMS    += mainwindow.ui

RESOURCES = cangaroo.qrc

include($$PWD/core/core.pri)
include($$PWD/driver/driver.pri)
include($$PWD/parser/dbc/dbc.pri)
include($$PWD/decoders/decoders.pri)
include($$PWD/window/TraceWindow/TraceWindow.pri)
include($$PWD/window/SetupDialog/SetupDialog.pri)
include($$PWD/window/LogWindow/LogWindow.pri)
include($$PWD/window/GraphWindow/GraphWindow.pri)
include($$PWD/window/CanStatusWindow/CanStatusWindow.pri)
include($$PWD/window/RawTxWindow/RawTxWindow.pri)
include($$PWD/window/TxGeneratorWindow/TxGeneratorWindow.pri)
include($$PWD/window/ReplayWindow/ReplayWindow.pri)
include($$PWD/window/ScriptWindow/ScriptWindow.pri)


unix:PKGCONFIG += libnl-3.0
unix:PKGCONFIG += libnl-route-3.0
unix:PKGCONFIG += python3-embed
unix:INCLUDEPATH += /usr/include/libnl3

# Try python3 first, fallback to python
PY_CMD = python3
win32 {
    PY_CMD = python
    CONFIG += link_pkgconfig
    PKGCONFIG += python3-embed
}

PYBIND_INCLUDES = $$system($$PY_CMD -m pybind11 --includes)
# Split the string by space and remove -I prefixes
PYBIND_INCLUDES_CLEAN = $$replace(PYBIND_INCLUDES, "-I", "")
INCLUDEPATH += $$PYBIND_INCLUDES_CLEAN

# Surgical fix for redundant /lib/lib path from pkg-config
unix:LIBS ~= s|/usr/lib/lib/|/usr/lib/|g
unix:include($$PWD/driver/SocketCanDriver/SocketCanDriver.pri)

include($$PWD/driver/CANBlastDriver/CANBlastDriver.pri)
include($$PWD/driver/SLCANDriver/SLCANDriver.pri)
include($$PWD/driver/GrIPDriver/GrIPDriver.pri)

win32:include($$PWD/driver/CandleApiDriver/CandleApiDriver.pri)

DISTFILES +=
