#!/bin/sh
# Install Flux Application (Full)

NAME="Flux Application (Full)"
INSTALL_PATH="$HOME/.config/QtProject/qtcreator/templates/wizards/$NAME"
SOURCE_PATH=$(dirname $0)

mkdir -p "$INSTALL_PATH"
cp -va "$SOURCE_PATH/$NAME"/* "$INSTALL_PATH"
