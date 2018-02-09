QT       += testlib qml

TARGET = unittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
ROOTDIR = $$PWD/../../

include(qpm.pri)
include($$ROOTDIR/app/MYPROJECT/MYPROJECT.pri)

DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$PWD/\\\"

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml

HEADERS +=     tests.h

write_file(../../qmlimport.path, QML_IMPORT_PATH)
