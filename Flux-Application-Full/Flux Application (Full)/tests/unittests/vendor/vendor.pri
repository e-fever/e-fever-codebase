
DEFINES += QPM_INIT\\\\(E\\\\)=\\"E.addImportPath(QStringLiteral(\\\\\\"qrc:/\\\\\\"));\\"
DEFINES += QPM_USE_NS
INCLUDEPATH += $$PWD
QML_IMPORT_PATH += $$PWD


include($$PWD/net/efever/snapshottesting/snapshottesting.pri)
include($$PWD/async/future/pri/asyncfuture.pri)
include($$PWD/com/github/benlau/qtshell/qtshell.pri)
include($$PWD/com/github/benlau/testable/testable.pri)
include($$PWD/net/efever/aconcurrent/aconcurrent.pri)
