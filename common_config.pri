# set variables for common configuration
CONFIG += c++11

isEmpty(BUILD_MODE) {
    error("We need BUILD_MODE variable, please insert it.")
}

ROOT_PATH=$(ROOT_PATH)
isEmpty(ROOT_PATH) {
    error("We need ROOT_PATH variable, please insert it.")
}

message( "[$$TARGET] BUILD_MODE=$$BUILD_MODE")
message( "[$$TARGET] ROOT_PATH=$$ROOT_PATH")

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11

CONFIG += $$BUILD_MODE
equals(BUILD_MODE, "debug") {
    QMAKE_CXXFLAGS += -g3
} else {
    QMAKE_CXXFLAGS += -Os
}

LOG_LEVEL_ERR = 1
LOG_LEVEL_WARN = 2
LOG_LEVEL_INFO = 3
LOG_LEVEL_DEBUG = 4

DEFINES += JSON_LIB_RAPIDJSON       # or JSON_LIB_HLOHMANN
