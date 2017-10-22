QT       += testlib qml

TARGET = MYPROJECTunittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$PWD/qmltests\\\"

ROOTDIR = $$PWD/../../

include(vendor/vendor.pri)
include($$ROOTDIR/MYPROJECT.pri)

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml

HEADERS +=     tests.h
