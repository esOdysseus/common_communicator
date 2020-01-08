TARGET = proto
TEMPLATE = lib
QT -= gui core

VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0
VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT

# for pre-process, if exist.
include ($$_PRO_FILE_PWD_/pre_proc.pri)


# for building
!include ($$_PRO_FILE_PWD_/../common_config.pri) {
    message( "Not exist common_config.pri file." )
}

CONFIG += shared
QMAKE_CXXFLAGS += -fPIC

DEFINES += LOGGER_TAG=\\\"PROTO\\\"
equals(BUILD_MODE, "debug") {
    DEFINES += LOG_DEBUG_MODE
    DEFINES += LOG_LEVEL=$$LOG_LEVEL_DEBUG
}

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/include/base    \
    $$_PRO_FILE_PWD_/include        \
    $$ROOT_PATH/lib/logger  \
    $$ROOT_PATH/lib/exception   \
    $$ROOT_PATH/lib

SOURCES += \
    $$files($$_PRO_FILE_PWD_/src/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/src/base/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/src/protocol/*.cpp)

# for installation.
EXTRA_BINFILES = \
    $$_PRO_FILE_PWD_/lib$$TARGET.so \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ.$$VER_MIN \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VERSION

PKG_CONFIG_DESCRIPTION=Protocols library that support common-communicator.

!include ($$_PRO_FILE_PWD_/../sdk_deploy.pri) {
    message( "Not exist sdk_deploy.pri file." )
}


# for post-process, if exist.
include ($$_PRO_FILE_PWD_/post_proc.pri)