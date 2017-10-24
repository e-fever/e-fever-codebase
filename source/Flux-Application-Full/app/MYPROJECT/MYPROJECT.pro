TEMPLATE = app

QT += qml quick
CONFIG += c++11

include(MYPROJECT.pri)

SOURCES += main.cpp

ROOTDIR = $$PWD/../..

# Default rules for deployment.
include(deployment.pri)

DISTFILES +=     qpm.json

