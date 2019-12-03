TARGET = communicator_example

TEMPLATE = subdirs

CONFIG += c++11 debug

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11 -g3 -Os

SUBDIRS += common_api


