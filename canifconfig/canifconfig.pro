TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += link_pkgconfig

SOURCES += main.c

PKGCONFIG += libnl-3.0
PKGCONFIG += libnl-route-3.0
