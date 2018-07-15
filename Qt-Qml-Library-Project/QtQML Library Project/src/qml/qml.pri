RESOURCES += \\
    $$PWD/%{ProjectName}.qrc

DEPENDPATH += $$PWD
QML_IMPORT_PATH += $$PWD

SOURCES += \\
    $$PWD/%{ProjectName}qmltypes.cpp

HEADERS += \\
    $$PWD/myprojectqmlplugin.h
