TARGET = communicator
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0
TEMPLATE = lib
VERSION = $$VER_MAJ"."$$VER_MIN"."$$VER_PAT

CONFIG += c++11 shared $$BUILD_MODE
CONFIG += create_pc create_prl no_install_prl

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11 -g3 -Os -fPIC

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/source/include/base    \
    $$_PRO_FILE_PWD_/source/include/implement    \
    $$_PRO_FILE_PWD_/source/lib/presentation   \
    $$_PRO_FILE_PWD_/source/lib

SOURCES += \
    $$files($$_PRO_FILE_PWD_/source/src/base/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/implement/server/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/src/implement/protocol/*.cpp)  \
    $$files($$_PRO_FILE_PWD_/source/lib/presentation/*.cpp)       \

# for installation.
HEADERS += \
    $$files($$_PRO_FILE_PWD_/api/common_api/*.h)    \
    $$files($$_PRO_FILE_PWD_/api/common_api/*.hpp)

EXTRA_BINFILES = \
    $$_PRO_FILE_PWD_/lib$$TARGET.so \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VER_MAJ.$$VER_MIN \
    $$_PRO_FILE_PWD_/lib$$TARGET.so.$$VERSION

!include ($$_PRO_FILE_PWD_/sdk_deploy.pri) {
    message( "Not exist sdk_deploy.pri file." )
}
