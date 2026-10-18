QT += charts

# QT += network
SUBDIRS += src
TEMPLATE = subdirs
CONFIG += ordered warn_on qt debug_and_release
CONFIG += c++20
LIBS += -lbsd
