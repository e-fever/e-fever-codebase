#!/bin/sh

set -v
set -e

PKG_PATH=${PWD}/deployment/linux

(cd $PKG_PATH; docker-compose build)


