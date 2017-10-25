QT       += testlib qml

TARGET = untitled
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\\\\"$$PWD/\\\\\\"
ROOTDIR = $$PWD/../../

include(vendor/vendor.pri)
include($$ROOTDIR/app/%{ProjectName}/%{ProjectName}.pri)

DEFINES += QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/\\\\\\"

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml

HEADERS +=     tests.h
