QT       += testlib qml

TARGET = %{ProjectName}unittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\\\\"$$PWD/\\\\\\"
DEFINES += QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/qmltests\\\\\\"

ROOTDIR = $$PWD/../../

include(vendor/vendor.pri)
include($$ROOTDIR/%{ProjectName}.pri)

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml

HEADERS +=     tests.h
