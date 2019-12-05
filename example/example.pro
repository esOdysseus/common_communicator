TARGET = communicator_example

TEMPLATE = subdirs

message( "[$$TARGET] BUILD_MODE=$$BUILD_MODE")
message( "[$$TARGET] DESTDIR=$$DESTDIR")


SUBDIRS += common_api


