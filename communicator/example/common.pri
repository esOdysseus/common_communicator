LIB_NAME=communicator

# Check validation
ROOT_PATH=$(ROOT_PATH)
isEmpty(ROOT_PATH) {
    error("We need ROOT_PATH variable, please insert it.")
}

!include ($$ROOT_PATH/common_config.pri) {
    error( "Not exist common_config.pri file." )
}

# Print configuration.
message( "[$$TARGET] [BUILD_MODE=$$BUILD_MODE]" )
message( "[$$TARGET] [DESTDIR=$$DESTDIR]" )
