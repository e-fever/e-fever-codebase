#!/bin/sh

ICON=$1
TMP_PATH=`mktemp -d`
ICON_PATH=$TMP_PATH/icon.iconset

mkdir -p ${ICON_PATH}

sips -z 16 16     ${ICON} --out ${ICON_PATH}/icon_16x16.png
sips -z 32 32     ${ICON} --out ${ICON_PATH}/icon_16x16@2x.png
sips -z 32 32     ${ICON} --out ${ICON_PATH}/icon_32x32.png
sips -z 64 64     ${ICON} --out ${ICON_PATH}/icon_32x32@2x.png
sips -z 128 128   ${ICON} --out ${ICON_PATH}/icon_128x128.png
sips -z 256 256   ${ICON} --out ${ICON_PATH}/icon_128x128@2x.png
sips -z 256 256   ${ICON} --out ${ICON_PATH}/icon_256x256.png
sips -z 512 512   ${ICON} --out ${ICON_PATH}/icon_256x256@2x.png
sips -z 512 512   ${ICON} --out ${ICON_PATH}/icon_512x512.png
cp ${ICON} ${ICON_PATH}/icon_512x512@2x.png
iconutil -c icns ${ICON_PATH}
cp ${TMP_PATH}/icon.icns $2


