CONFIG += c++20

SOURCES += \
    $$PWD/GrIPInterface.cpp \
    $$PWD/GrIPDriver.cpp \
    $$PWD/GrIP/CRC.c \
    $$PWD/GrIP/GrIP.cpp \
    $$PWD/GrIP/Protocol.cpp \
    $$PWD/GrIP/GrIPHandler.cpp

HEADERS  += \
    $$PWD/GrIPInterface.h \
    $$PWD/GrIPDriver.h \
    $$PWD/GrIP/CRC.h \
    $$PWD/GrIP/GrIP.h \
    $$PWD/GrIP/Protocol.h \
    $$PWD/GrIP/GrIPHandler.h

FORMS +=

