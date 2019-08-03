QT       += testlib qml qmltest

TARGET = qmltests
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += warn_on qmltestcase

TEMPLATE = app

QML_IMPORT_PATH += $$PWD

!win32 {
    QMAKE_CXXFLAGS += -Werror
}

SOURCES += \
    main.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += TEST_SOURCE_DIR=\\\"$$PWD\\\"

DISTFILES += \
    tst_Sample.qml

include($$ROOTDIR/cpp/cpp.pri)
include($$ROOTDIR/qml/qml.pri)
include($$ROOTDIR/vendor/vendor.pri)

HEADERS += \
    XBacktrace.h
