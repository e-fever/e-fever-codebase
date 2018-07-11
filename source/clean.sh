#!/bin/sh

git clean -f
find . -name vendor -exec rm -rf {} \;
