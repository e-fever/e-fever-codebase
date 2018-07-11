#!/bin/sh
# clean the source tree before export

git clean -f
find . -name "build-*" -exec rm -rf {} \;
find . -name vendor -exec rm -rf {} \;
find . -name node_modules -exec rm -rf {} \;
find . -name qmlimport.path -exec rm  {} \;
