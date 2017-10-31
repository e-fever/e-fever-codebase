QT       += testlib qml qmltest

TARGET = PROJECT
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += warn_on qmltestcase

TEMPLATE = app

IMPORTPATH = $$PWD

SOURCES += \
    main.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += TEST_SOURCE_DIR=\\\"$$PWD\\\"

DISTFILES += \
    tst_Sample.qml
