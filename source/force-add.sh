#!/bin/sh

find . -name ".git" -exec rm -rf {} \;
find .. -name vendor ! -path "../source/*" ! -path "../default/*" -exec git add -f {} \;
