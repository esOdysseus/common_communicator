# set variables for common configuration
CONFIG += c++11

isEmpty(BUILD_MODE) {
    error("We need BUILD_MODE variable, please insert it.")
}

message( "[$$TARGET] BUILD_MODE=$$BUILD_MODE")

QMAKE_CXXFLAGS += -ftemplate-depth=3000
QMAKE_CXXFLAGS += -std=c++11

CONFIG += $$BUILD_MODE
equals(BUILD_MODE, "debug") {
    QMAKE_CXXFLAGS += -g3
} else {
    QMAKE_CXXFLAGS += -Os
}
