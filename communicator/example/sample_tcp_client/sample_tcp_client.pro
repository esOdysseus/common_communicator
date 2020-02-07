TARGET = sample_tcp_client
TEMPLATE = app
QT -= gui core

isEmpty(DESTDIR) {
    error("We need DESTDIR variable, please insert it.")
}

message( "[$$TARGET] DESTDIR=$$DESTDIR")
!include ($$DESTDIR/../../common_config.pri) {
    error( "Not exist common_config.pri file." )
}

# for building
LIBS += -lpthread -lcommunicator -L$$DESTDIR/../lib

INCLUDEPATH += \
    $$_PRO_FILE_PWD_    \
    $$DESTDIR/../include

SOURCES += \
    $$files($$_PRO_FILE_PWD_/*.cpp)  


# for installation.
HEADERS += \

EXTRA_BINFILES += $$_PRO_FILE_PWD_/$$TARGET

