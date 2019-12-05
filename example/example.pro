TARGET = communicator_example

TEMPLATE = subdirs

message( "[$$TARGET] BUILD_MODE=$$BUILD_MODE")
message( "[$$TARGET] DESTDIR=$$DESTDIR")


CONFIG += c++11 $$BUILD_MODE

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11 -g3 -Os


SUBDIRS += common_api


