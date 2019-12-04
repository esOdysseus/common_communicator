TARGET = communicator
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0
TEMPLATE = lib
VERSION = $$VER_MAJ.$$VER_MIN.$$VER_PAT

# QT += qml quick widgets

CONFIG += c++11 shared debug

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11 -g3 -Os -fPIC

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/source/Include/base    \
    $$_PRO_FILE_PWD_/source/Include/implement    \
    $$_PRO_FILE_PWD_/source/Lib/presentation   \
    $$_PRO_FILE_PWD_/source/Lib

SOURCES += \
    $$files($$PWD/source/Src/base/*.cpp)  \
    $$files($$PWD/source/Src/implement/server/*.cpp)  \
    $$files($$PWD/source/Src/implement/protocol/*.cpp)  \
    $$files($$PWD/source/Lib/presentation/*.cpp)       \

HEADERS += \

DESTDIR=$$_PRO_FILE_PWD_/api/
EXTRA_BINFILES += \
    $$_PRO_FILE_PWD_/debug/lib$$TARGET.so \
    $$_PRO_FILE_PWD_/debug/lib$$TARGET.so.$$VER_MAJ \
    $$_PRO_FILE_PWD_/debug/lib$$TARGET.so.$$VER_MAJ.$$VER_MIN \
    $$_PRO_FILE_PWD_/debug/lib$$TARGET.so.$$VERSION

