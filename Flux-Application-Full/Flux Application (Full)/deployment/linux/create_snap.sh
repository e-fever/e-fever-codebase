#!/bin/sh

set -v
set -e

if [ -z "$BUILD_DIR"]
then
    BUILD_DIR=${PWD}/build-release
fi

ROOT_DIR=${PWD}
ARCHIVE_DIR=${PWD}/archive
SRC_DIR=${PWD}/qml
PRO=${ROOT_DIR}/app/app.pro
PKG=${BUILD_DIR}/pkg
APP=${PKG}/app

QTDIR=$(dirname $(which qmake))/..

rm -rf "$BUILD_DIR"
rm -rf "$BUILD_DIR/pkg"

mkdir -p "$BUILD_DIR"
mkdir -p "$BUILD_DIR/pkg/snap"
mkdir -p "$BUILD_DIR/pkg/bin"
mkdir -p ${APP}/bin

mkdir -p "$ARCHIVE_DIR"
cd "$BUILD_DIR"

# Remove tst package from vendor
find ${ROOT_DIR}/cpp -name "tst_*.qml" -exec rm {} \\;
qmake ${PRO} -r
make

linuxdeployqt $BUILD_DIR/%{ProjectName} -qt $QTDIR -s ${SRC_DIR} --appdir $APP

cp ${ROOT_DIR}/deployment/linux/qt5-launch ${APP}/bin
chmod u+x ${APP}/bin/qt5-launch
cp ${ROOT_DIR}/deployment/linux/snapcraft.yaml ${BUILD_DIR}/pkg/snap
cp ${ROOT_DIR}/deployment/linux/%{ProjectName}.desktop ${APP}
cp ${ROOT_DIR}/deployment/linux/icon.png ${APP}

cd ${BUILD_DIR}/pkg; snapcraft clean; snapcraft snap
find . -name "*.snap" -exec cp {} "${ARCHIVE_DIR}" \\;
