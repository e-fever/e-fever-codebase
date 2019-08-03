QT += testlib qml quick
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

ROOTDIR = $$absolute_path($$PWD/../..)

include($$ROOTDIR/cpp/cpp.pri)
include($$ROOTDIR/qml/qml.pri)
include($$ROOTDIR/vendor/vendor.pri)

SOURCES +=  \
    tst_Sample.cpp
