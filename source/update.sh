#!/bin/sh

for i in `ls`
do
    if [ -d "$i" ]
    then
        (cd $i; qtcwizard pack-installer "../../$i")
    fi
done
