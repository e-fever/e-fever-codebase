TEMPLATE = app

QT += qml quick
CONFIG += c++11

QT += quick qml

SOURCES += main.cpp

SOURCES += $$PWD/appview.cpp

RESOURCES += \
    $$PWD/PROJECT.qrc

INCLUDEPATH += $$PWD

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH += $$PWD

HEADERS += \
    $$PWD/appview.h

ROOT_DIR = $$PWD

# Default rules for deployment.
include(deployment.pri)
include(qpm.pri)

DISTFILES += \
    qpm.json \
    qpm.pri

