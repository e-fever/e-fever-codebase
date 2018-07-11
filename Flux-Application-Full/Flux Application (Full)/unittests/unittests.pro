QT       += testlib qml

TARGET = unittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp     tests.cpp

DEFINES += SRCDIR=\\\\\\"$$PWD/\\\\\\"
ROOTDIR = $$absolute_path($$PWD/..)

include(qpm.pri)
include($$ROOTDIR/cpp/cpp.pri)
include($$ROOTDIR/qml/qml.pri)

DEFINES += QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/\\\\\\" ROOTDIR=\\\\\\"$$ROOTDIR/\\\\\\"

DISTFILES +=     qpm.json     qmltests/tst_QmlTests.qml

HEADERS += \\    
    testcases.h

write_file(../../qmlimport.path, QML_IMPORT_PATH)
