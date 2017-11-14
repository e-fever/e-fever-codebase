TEMPLATE = app
TARGET = tst_example1
CONFIG += warn_on qmltestcase
SOURCES += main.cpp
DEFINES += QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/\\\\\\"

DISTFILES += \\
    tst_demo1.qml \\
    README.md \\
    CustomItem.qml

include(../../snapshottesting.pri)

#Run `qpm install` on source's top most folder to obtain required library
include(../../vendor/vendor.pri)

