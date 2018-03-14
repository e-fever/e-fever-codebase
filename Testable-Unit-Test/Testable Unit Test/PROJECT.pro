QT       += testlib qml

TARGET = %{ProjectName}
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \\
    main.cpp \\
    tests.cpp

DEFINES += SRCDIR=\\\\\\"$$PWD/\\\\\\" QUICK_TEST_SOURCE_DIR=\\\\\\"$$PWD/qmltests\\\\\\"

include(qpm.pri)

DISTFILES += \\
    qpm.json \\
    qmltests/tst_QmlTests.qml

HEADERS += \\
    tests.h

