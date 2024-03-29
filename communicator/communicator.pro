TARGET = communicator
TEMPLATE = lib
QT -= gui core

VER_MAJ = 0
VER_MIN = 4
VER_PAT = 1
VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT

message( "[ Build 'communicator' " )

ROOT_PATH=$(ROOT_PATH)
isEmpty(ROOT_PATH) {
    error("We need ROOT_PATH variable, please insert it.")
}

# for pre-process, if exist.
!include ($$_PRO_FILE_PWD_/pre_proc.pri) {
    message( "Not exist pre_proc.pri file." )
}

!include ($$ROOT_PATH/common_config.pri) {
    message( "Not exist common_config.pri file." )
}

!include ($$ROOT_PATH/pkg_config.pri) {
    message( "Not exist pkg_config.pri file." )
}

# for building
CONFIG += shared
QMAKE_CXXFLAGS += -fPIC
QMAKE_CFLAGS += -fPIC

DEFINES += LOGGER_TAG=\\\"COMM\\\"
DEFINES += VER_MAJ=$$VER_MAJ
DEFINES += VER_MIN=$$VER_MIN
DEFINES += VER_PAT=$$VER_PAT

equals(CPU_ARCH,"x86") {
    # for logger_mode (default logger == DLT logger)
    DEFINES += LOG_MODE_STDOUT
}

!contains(DEFINES, LOG_MODE_STDOUT) {
    DEFINES += LOG_DLT_CID=\\\"comm\\\"
}

equals(BUILD_MODE, "debug") {
    DEFINES += LOG_DEBUG_MODE
    DEFINES += LOG_LEVEL=$$LOG_LEVEL_DEBUG
}

equals(BUILD_MODE, "release") {
    DEFINES += LOG_LEVEL=$$LOG_LEVEL_INFO
}

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/api/include    \
    $$_PRO_FILE_PWD_/source/include/base    \
    $$_PRO_FILE_PWD_/source/include/implement    \
    $$ROOT_PATH/lib     \
    $$ROOT_PATH/lib/logger  \
    $$ROOT_PATH/lib/json    \
    $$ROOT_PATH/lib/exception   \
    $$ROOT_PATH/lib/lock    \
    $$ROOT_PATH/lib/time    \
    $$ROOT_PATH/lib/util

!contains(DEFINES, LOG_MODE_STDOUT) {
    INCLUDEPATH += $$ROOT_PATH/lib/dlt
    INCLUDEPATH += $$get_incs_pkgconfig(automotive-dlt)
}

!contains(DEFINES, LOG_MODE_STDOUT) {
    SOURCES += $$files($$ROOT_PATH/lib/dlt/*.cpp)
}
SOURCES += \
    $$files($$_PRO_FILE_PWD_/api/src/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/base/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/implement/provider/*.cpp)  \
    $$files($$ROOT_PATH/lib/json/*.cpp)     \
    $$files($$ROOT_PATH/lib/pal/*.c)        \
    $$files($$ROOT_PATH/lib/time/*.cpp)        \
    $$files($$ROOT_PATH/lib/util/*.cpp)

LIBS += -ldl -lpthread
!contains(DEFINES, LOG_MODE_STDOUT) {
    LIBS += $$get_libs_pkgconfig(automotive-dlt)
}

# for installation.
HEADERS += \
    $$files($$_PRO_FILE_PWD_/api/include/*.h)    \
    $$files($$_PRO_FILE_PWD_/api/include/*.hpp)

EXTRA_BINFILES = \
    $$_PRO_FILE_PWD_/lib$$TARGET.so \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ.$$VER_MIN \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VERSION

PKG_CONFIG_DESCRIPTION=Common-communicator SDK library that support transaction-orient & service-orient.

!include ($$ROOT_PATH/sdk_deploy.pri) {
    message( "Not exist sdk_deploy.pri file." )
}

# for process, if exist.
!include ($$_PRO_FILE_PWD_/post_proc.pri) {
    message( "Not exist post_proc.pri file." )
}
