SOURCES += \
    $$PWD/CanInterface.cpp \
    $$PWD/CanInterfaceProvider.cpp \
    $$PWD/socketcan/libsocketcan/libsocketcan.c \
    $$PWD/socketcan/SocketCanInterface.cpp \
    $$PWD/socketcan/SocketCanInterfaceProvider.cpp

HEADERS  += \
    $$PWD/CanInterface.h \
    $$PWD/CanInterfaceProvider.h \
    $$PWD/socketcan/libsocketcan/libsocketcan.h \
    $$PWD/socketcan/libsocketcan/can_netlink.h \
    $$PWD/socketcan/SocketCanInterface.h \
    $$PWD/socketcan/SocketCanInterfaceProvider.h
