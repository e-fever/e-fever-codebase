TARGET = MYPROJECT
TEMPLATE = lib

#Remove this line if you are building a non-QML library
QT += quick

ROOT_DIR = $$absolute_path($$PWD/..)

isEmpty(SHARED): SHARED = "false"
isEmpty(PLUGIN): PLUGIN = "false"

DEFAULT_INSTALL_ROOT = $$[QT_INSTALL_LIBS]

isEmpty(INSTALL_ROOT) {
    INSTALL_ROOT = $${DEFAULT_INSTALL_ROOT}
}

include($$ROOT_DIR/src/MYPROJECT.pri)

INSTALLS += target

equals(PLUGIN, "true") {
    SHARED = "true"
    TARGET = MYPROJECTqmlplugin
    isEmpty(INSTALL_ROOT): INSTALL_ROOT=$$[QT_INSTALL_QML]

    target.path = $${INSTALL_ROOT}/MYPACKAGE

    QML.files = $$PWD/qmldir $$PWD/ MYPROJECT.qmltypes
    QML.path = $${INSTALL_ROOT}/MYPACKAGE

    INSTALLS += QML

} else {
    isEmpty(INSTALL_ROOT): INSTALL_ROOT=$$[QT_INSTALL_LIBS]

    # Uncomment the following lines to install header file
    #headers.files += $$ROOT_DIR/src/xxx.h
    target.path = $${INSTALL_ROOT}/lib
    INSTALLS += headers

    headers.path = $${INSTALL_ROOT}/include
}

equals(SHARED, "false") {
    CONFIG += staticlib
}

