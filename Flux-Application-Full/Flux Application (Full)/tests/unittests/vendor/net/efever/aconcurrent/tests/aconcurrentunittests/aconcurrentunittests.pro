QT       += testlib qml

TARGET = aconcurrent
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=     main.cpp \\    
    aconcurrenttests.cpp

DEFINES += SRCDIR=\\\\\\"$$PWD/\\\\\\"
DEFINES += QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/\\\\\\"

ROOTDIR = $$PWD/../../

include(vendor/vendor.pri)
include($$ROOTDIR/aconcurrent.pri)

DISTFILES +=     qpm.json \\    
    ../../README.md \\
    ../../qpm.json \\
    ../../appveyor.yml

HEADERS += \\    
    aconcurrenttests.h
