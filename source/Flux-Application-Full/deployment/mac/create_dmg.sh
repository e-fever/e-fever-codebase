#!/bin/sh

set -v
set -e

BUILD_DIR=${PWD}/build-release
PRO=${PWD}/app/app.pro
PKG_PATH=${PWD}/deployment/mac
ICON=${PWD}/deployment/mac/icon.icns
APP=${BUILD_DIR}/MYPROJECT.app
QML_DIR=${PWD}/qml

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

qmake "${PRO}"
make
cp ${PKG_PATH}/dmg.json $BUILD_DIR
cp ${ICON} $BUILD_DIR
macdeployqt "${APP}" -verbose=1 -qmldir="${QML_DIR}" -appstore-compliant
appdmg dmg.json MYPROJECT.dmg
