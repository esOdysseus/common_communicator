TARGET = sample_tcp_client
TEMPLATE = app
QT -= gui core

!include ($$_PRO_FILE_PWD_/../common.pri) {
    error( "Not exist common.pri file." )
}

isEmpty(DESTDIR) {
    error("We need DESTDIR variable, please insert it.")
}

# for building
LIBS += -l$$LIB_NAME -L$$DESTDIR/../lib

equals(CPU_ARCH, "x86") {
    # If OS-type is Ubuntu, then need these libraries.
    LIBS += -lpthread
}

INCLUDEPATH += \
    $$_PRO_FILE_PWD_    \
    $$DESTDIR/../include

SOURCES += \
    $$files($$_PRO_FILE_PWD_/*.cpp)  


# for installation.
HEADERS += \

EXTRA_BINFILES += $$_PRO_FILE_PWD_/$$TARGET

