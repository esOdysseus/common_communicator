

# copy base-code from communicator to protocol-base folder.
COMM_ROOT=$$_PRO_FILE_PWD_/../communicator
TO_COPY_HEADERS += \
    $$files($$COMM_ROOT/source/include/base/CPayload.h)    \
    $$files($$COMM_ROOT/source/include/base/CRawMessage.h)    \
    $$files($$COMM_ROOT/source/include/base/IAliasPVD.h)    \
    $$files($$COMM_ROOT/source/include/base/IProtocolInf.h)    \
    $$files($$COMM_ROOT/source/include/base/Enum_common.h)  \
    $$files($$COMM_ROOT/source/include/base/CSource.h)
TO_COPY_CPPS += \
    $$files($$COMM_ROOT/source/src/base/CPayload.cpp)      \
    $$files($$COMM_ROOT/source/src/base/CRawMessage.cpp)    \
    $$files($$COMM_ROOT/source/src/base/IAliasPVD.cpp)    \
    $$files($$COMM_ROOT/source/src/base/IProtocolInf.cpp)    \
    $$files($$COMM_ROOT/source/src/base/CSource.cpp)
PROTOCOL_PATH=$${_PRO_FILE_PWD_}

# make command & register to pre-linker
mkdir.commands = $(MKDIR) -p $${PROTOCOL_PATH}/include/base && $(MKDIR) -p $${PROTOCOL_PATH}/src/base
QMAKE_PRE_LINK += $$system(mkdir -p $${PROTOCOL_PATH}/include/base)
QMAKE_PRE_LINK += $$system(mkdir -p $${PROTOCOL_PATH}/src/base)
for(FILE, TO_COPY_HEADERS){
    QMAKE_PRE_LINK += $$system(cp $${FILE} $${PROTOCOL_PATH}/include/base/)
}
for(FILE, TO_COPY_CPPS){
    QMAKE_PRE_LINK += $$system(cp $${FILE} $${PROTOCOL_PATH}/src/base/)
}
