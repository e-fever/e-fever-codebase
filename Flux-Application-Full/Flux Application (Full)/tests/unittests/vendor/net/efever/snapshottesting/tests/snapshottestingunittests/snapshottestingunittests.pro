QT       += testlib qml concurrent

TARGET = snapshottesting
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp \\    
    snapshottests.cpp

DEFINES += SRCDIR=\\\\\\"$$PWD/\\\\\\"
DEFINES += QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/\\\\\\"

ROOTDIR = $$PWD/../../

include(vendor/vendor.pri)
include($$ROOTDIR/snapshottesting.pri)

DISTFILES +=     qpm.json     \\
    ../../qpm.json \\
    sample/red-100x100.png \\
    sample/Sample1.qml \\
    sample/Sample2.qml \\
    sample/Sample3.qml \\
    sample/Sample4.qml \\
    qmltests/tst_SnapshotTesting.qml \\
    snapshots.json \\
    ../../README.md \\
    sample/Sample5Form.ui.qml \\
    sample/Sample5.qml \\
    sample/Sample6.qml \\
    sample/Sample7.qml \\
    sample/Sample8.qml \\
    sample/Sample9.qml \\
    sample/Sample_QJSValue.qml \\
    sample/Sample_Loader_Async.qml

HEADERS += \\    
    snapshottests.h
