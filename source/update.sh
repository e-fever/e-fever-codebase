#!/bin/sh

# Remove .gitignore in vendor folder
_IFS=$IFS
IFS=''
find .. -name "vendor" -type d | while read data
do 
   find "$data" -name .gitignore -exec rm {} \;
done

IFS=$_IFS

for i in `ls`
do
    if [ -f "$i/wizard.json"  ]
    then
        (cd $i; qtcwizard pack-installer "../../$i")
    fi
done

