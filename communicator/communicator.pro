TARGET = communicator
TEMPLATE = lib
QT -= gui core

VER_MAJ = 0
VER_MIN = 1
VER_PAT = 6
VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT

!include ($$_PRO_FILE_PWD_/../common_config.pri) {
    message( "Not exist common_config.pri file." )
}

# for building
CONFIG += shared
QMAKE_CXXFLAGS += -fPIC
QMAKE_CFLAGS += -fPIC
LIBS += -ldl -lpthread

DEFINES += LOGGER_TAG=\\\"COMM\\\"
DEFINES += VER_MAJ=$$VER_MAJ
DEFINES += VER_MIN=$$VER_MIN
DEFINES += VER_PAT=$$VER_PAT

equals(BUILD_MODE, "debug") {
    DEFINES += LOG_DEBUG_MODE
    DEFINES += LOG_LEVEL=$$LOG_LEVEL_DEBUG
}

equals(BUILD_MODE, "release") {
    DEFINES += LOG_LEVEL=$$LOG_LEVEL_INFO
}

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/source/include/base    \
    $$_PRO_FILE_PWD_/source/include/implement    \
    $$ROOT_PATH/lib     \
    $$ROOT_PATH/lib/logger  \
    $$ROOT_PATH/lib/json    \
    $$ROOT_PATH/lib/exception   \
    $$ROOT_PATH/lib/lock

SOURCES += \
    $$files($$_PRO_FILE_PWD_/source/src/base/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/implement/server/*.cpp)  \
    $$files($$ROOT_PATH/lib/json/*.cpp)     \
    $$files($$ROOT_PATH/lib/pal/*.c)

# for installation.
HEADERS += \
    $$files($$_PRO_FILE_PWD_/api/common_api/*.h)    \
    $$files($$_PRO_FILE_PWD_/api/common_api/*.hpp)

EXTRA_BINFILES = \
    $$_PRO_FILE_PWD_/lib$$TARGET.so \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ.$$VER_MIN \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VERSION

PKG_CONFIG_DESCRIPTION=Common-communicator SDK library that support transaction-orient & service-orient.

!include ($$_PRO_FILE_PWD_/../sdk_deploy.pri) {
    message( "Not exist sdk_deploy.pri file." )
}

# for process, if exist.
include ($$_PRO_FILE_PWD_/post_proc.pri)
