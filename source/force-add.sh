#!/bin/sh

find .. -name vendor ! -path "../source/*" ! -path "../default/*" -exec git add -f {} \;
