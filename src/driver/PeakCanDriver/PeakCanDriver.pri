SOURCES += \
    $$PWD/PeakCanDriver.cpp \
    $$PWD/PeakCanInterface.cpp \
    $$PWD/PeakCanSetupPage.cpp

win32:LIBS += -L$$PWD/pcan-basic-api/Win32/VC_LIB/ -lPCANBasic
win32:INCLUDEPATH += $$PWD/pcan-basic-api/Win32/VC_LIB
win32:DEPENDPATH += $$PWD/pcan-basic-api/Win32/VC_LIB

HEADERS += \
    $$PWD/PeakCanDriver.h \
    $$PWD/PeakCanInterface.h \
    $$PWD/PeakCanSetupPage.h

FORMS += \
    $$PWD/PeakCanSetupPage.ui
