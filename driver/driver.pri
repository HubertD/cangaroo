SOURCES += \
    $$PWD/CanInterface.cpp \
    $$PWD/socketcan/libsocketcan/libsocketcan.c \
    $$PWD/socketcan/SocketCanInterface.cpp \
    $$PWD/CanListener.cpp \
    $$PWD/socketcan/SocketCanDriver.cpp \
    $$PWD/CanDriver.cpp

HEADERS  += \
    $$PWD/CanInterface.h \
    $$PWD/socketcan/libsocketcan/libsocketcan.h \
    $$PWD/socketcan/libsocketcan/can_netlink.h \
    $$PWD/socketcan/SocketCanInterface.h \
    $$PWD/CanListener.h \
    $$PWD/socketcan/SocketCanDriver.h \
    $$PWD/CanDriver.h
