SOURCES += \
    $$PWD/CandleApiDriver.cpp \
    $$PWD/CandleApiSetupPage.cpp \
    $$PWD/CandleApiInterface.cpp
#    $$PWD/CandlelApiInterface.cpp \

win32:LIBS += -L$$PWD/api/ -lcandle_api

HEADERS += \
    $$PWD/api/candle.h \
    $$PWD/CandleApiDriver.h \
#    $$PWD/PeakCanInterface.h \
    $$PWD/CandleApiSetupPage.h \
    $$PWD/CandleApiInterface.h

FORMS += \
    $$PWD/CandleApiSetupPage.ui
#    $$PWD/PeakCanSetupPage.ui
