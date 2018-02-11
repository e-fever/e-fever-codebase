QT       += testlib qml

TARGET = unittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$PWD/qmltests\\\"

ROOTDIR = $$PWD/../../

include(qpm.pri)
include($$ROOTDIR/MYPROJECT.pri)

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml \
    ../../.travis.yml \
    ../../README.md \
    ../../appveyor.yml \
    qpm.pri

HEADERS +=     tests.h \
    XBacktrace.h \
    XBacktrace.h

!win32 {
    QMAKE_CXXFLAGS += -Werror
}
