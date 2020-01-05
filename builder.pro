TEMPLATE = subdirs
CONFIG += ordered

equals(TARGET, "communicator") {
    SUBDIRS += communicator
}

equals(TARGET, "protocol") {
    SUBDIRS += protocol
}

equals(TARGET, "example") {
    SUBDIRS += communicator/example
}

DISTFILES += \