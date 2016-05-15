
# these are all qt libraries required for edbee
QT = core gui widgets
TEMPLATE = app


# the sources of this project
SOURCES = example2.cpp

# include the edbee project include file 
include(../edbee-lib/edbee-lib/edbee-lib.pri)
INCLUDEPATH += ../diff-match-patch-cpp-stl
HEADERS +=

