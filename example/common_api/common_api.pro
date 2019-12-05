TARGET = example_common_api

TEMPLATE = app

message( "[$$TARGET] BUILD_MODE=$$BUILD_MODE")
message( "[$$TARGET] DESTDIR=$$DESTDIR")

!include ($$DESTDIR/../../common_config.pri) {
    message( "Not exist common_config.pri file." )
}

CONFIG += $$BUILD_MODE
equals(BUILD_MODE, "debug") {
    QMAKE_CXXFLAGS += -g3
} else {
    QMAKE_CXXFLAGS += -Os
}

LIBS += -lcommunicator -L$$DESTDIR/../lib

INCLUDEPATH += \
    $$_PRO_FILE_PWD_    \
    $$DESTDIR/../include

SOURCES += \
    $$files($$_PRO_FILE_PWD_/*.cpp)  


# for installation.
HEADERS += \

EXTRA_BINFILES += $$_PRO_FILE_PWD_/$$TARGET

