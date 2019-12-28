TARGET = communicator
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 1
TEMPLATE = lib
VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT

!include ($$_PRO_FILE_PWD_/common_config.pri) {
    message( "Not exist common_config.pri file." )
}

# for building
CONFIG += shared
QMAKE_CXXFLAGS += -fPIC

equals(BUILD_MODE, "debug") {
    DEFINES += LOG_DEBUG_MODE
}
DEFINES += JSON_LIB_RAPIDJSON       # or JSON_LIB_HLOHMANN

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/source/include/base    \
    $$_PRO_FILE_PWD_/source/include/implement    \
    $$_PRO_FILE_PWD_/source/lib/logger  \
    $$_PRO_FILE_PWD_/source/lib/json    \
    $$_PRO_FILE_PWD_/source/lib/exception

SOURCES += \
    $$files($$_PRO_FILE_PWD_/source/src/base/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/implement/server/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/implement/protocol/*.cpp)   \
    $$files($$_PRO_FILE_PWD_/source/lib/json/*.cpp)

# for installation.
DESTDIR=$$_PRO_FILE_PWD_/$$BUILD_MODE/lib

HEADERS += \
    $$files($$_PRO_FILE_PWD_/api/common_api/*.h)    \
    $$files($$_PRO_FILE_PWD_/api/common_api/*.hpp)

EXTRA_BINFILES = \
    $$_PRO_FILE_PWD_/lib$$TARGET.so \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ.$$VER_MIN \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VERSION

PKG_CONFIG_DESCRIPTION=Common-communicator SDK library that support transaction-orient & service-orient.

!include ($$_PRO_FILE_PWD_/sdk_deploy.pri) {
    message( "Not exist sdk_deploy.pri file." )
}
