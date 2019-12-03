TARGET = example_common_api

TEMPLATE = app

# QT += qml quick widgets

CONFIG += c++11 debug

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11 -g3 -Os


# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

LIBS += -lcommunicator -L$$_PRO_FILE_PWD_/../../delivery

INCLUDEPATH += \
    $$_PRO_FILE_PWD_    \
    $$_PRO_FILE_PWD_/../../delivery/api/common_api

SOURCES += \
    $$files($$PWD/*.cpp)  

HEADERS += \

