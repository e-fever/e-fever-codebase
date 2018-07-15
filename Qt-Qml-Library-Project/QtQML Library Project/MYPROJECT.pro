TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += buildlib
SUBDIRS += tests

tests.depends = builddlib
