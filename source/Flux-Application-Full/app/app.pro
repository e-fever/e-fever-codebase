TEMPLATE = app

QT += qml quick
CONFIG += c++11

SOURCES += main.cpp

ROOTDIR = $$absolute_path($$PWD/..)

include(qpm.pri)
include($$ROOTDIR/cpp/cpp.pri)
include($$ROOTDIR/qml/qml.pri)

# Default rules for deployment.
include(deployment.pri)

DISTFILES +=     qpm.json

