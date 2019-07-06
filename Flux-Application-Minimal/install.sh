#!/bin/sh
# Install Flux Application (Minimal)

NAME="Flux Application (Minimal)"
INSTALL_PATH="$HOME/.config/QtProject/qtcreator/templates/wizards/$NAME"
SOURCE_PATH=$(dirname $0)

mkdir -p "$INSTALL_PATH"
cp -va "$SOURCE_PATH/$NAME"/* "$INSTALL_PATH"
