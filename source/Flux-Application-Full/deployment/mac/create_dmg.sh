#!/bin/sh

set -v
set -e

BUILD_DIR=${PWD}/build-release
PRO=${PWD}/app/app.pro
PKG_PATH=${PWD}/deployment/mac
ICON=${PWD}/deployment/mac/icon.icns

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

qmake "${PRO}"
make
cp ${PKG_PATH}/dmg.json $BUILD_DIR
cp ${ICON} $BUILD_DIR
appdmg dmg.json MYPROJECT.dmg
