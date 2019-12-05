TARGET = example_common_api

TEMPLATE = app

message( "[$$TARGET] BUILD_MODE=$$BUILD_MODE")
message( "[$$TARGET] DESTDIR=$$DESTDIR")

CONFIG += c++11 $$BUILD_MODE

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11 -g3 -Os

LIBS += -lcommunicator -L$$DESTDIR/../lib

INCLUDEPATH += \
    $$_PRO_FILE_PWD_    \
    $$DESTDIR/../include

SOURCES += \
    $$files($$_PRO_FILE_PWD_/*.cpp)  


# for installation.
HEADERS += \

EXTRA_BINFILES += $$_PRO_FILE_PWD_/$$TARGET

