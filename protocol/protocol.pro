TARGET = proto
TEMPLATE = lib
QT -= gui core

VER_MAJ = 0
VER_MIN = 3
VER_PAT = 0
VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT

# for pre-process, if exist.
!include ($$_PRO_FILE_PWD_/pre_proc.pri) {
    message( "Not exist pre_proc.pri file." )
}

# for building
!include ($$_PRO_FILE_PWD_/../common_config.pri) {
    message( "Not exist common_config.pri file." )
}

!include ($$_PRO_FILE_PWD_/../pkg_config.pri) {
    message( "Not exist pkg_config.pri file." )
}

CONFIG += shared
QMAKE_CXXFLAGS += -fPIC
QMAKE_CFLAGS += -fPIC

DEFINES += LOGGER_TAG=\\\"PROTO\\\"

equals(CPU_ARCH,"x86") {
    # for logger_mode (default logger == DLT logger)
    DEFINES += LOG_MODE_STDOUT
}

!contains(DEFINES, LOG_MODE_STDOUT) {
    DEFINES += LOG_DLT_CID=\\\"prot\\\"
}

equals(BUILD_MODE, "debug") {
    DEFINES += LOG_DEBUG_MODE
    DEFINES += LOG_LEVEL=$$LOG_LEVEL_DEBUG
}

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/include/base    \
    $$_PRO_FILE_PWD_/include        \
    $$ROOT_PATH/lib/logger  \
    $$ROOT_PATH/lib/exception   \
    $$ROOT_PATH/lib/lock   \
    $$ROOT_PATH/lib/time   \
    $$ROOT_PATH/lib

!contains(DEFINES, LOG_MODE_STDOUT) {
    INCLUDEPATH += $$ROOT_PATH/lib/dlt
    INCLUDEPATH += $$get_incs_pkgconfig(automotive-dlt)
}

SOURCES += \
    $$files($$_PRO_FILE_PWD_/src/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/src/base/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/src/protocol/*.cpp)    \
    $$files($$ROOT_PATH/lib/time/*.cpp)

!contains(DEFINES, LOG_MODE_STDOUT) {
    SOURCES += $$files($$ROOT_PATH/lib/dlt/*.cpp)
}

!contains(DEFINES, LOG_MODE_STDOUT) {
    LIBS += $$get_libs_pkgconfig(automotive-dlt)
}

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
!include ($$_PRO_FILE_PWD_/post_proc.pri) {
    message( "Not exist post_proc.pri file." )
}
