INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
QML_IMPORT_PATH += $$PWD/qml

QT       += qml-private qml quick

RESOURCES += \\
    $$PWD/snapshottesting.qrc

HEADERS += \\
    $$PWD/snapshottesting.h \\
    $$PWD/snapshottestingadapter.h \\
    $$PWD/private/snapshottesting_p.h

SOURCES += \\
    $$PWD/snapshottesting.cpp \\
    $$PWD/snapshottestingadapter.cpp
