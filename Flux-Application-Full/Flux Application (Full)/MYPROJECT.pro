TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += app/%{ProjectName}/%{ProjectName}.pro \\
           tests/%{ProjectName}unittests
