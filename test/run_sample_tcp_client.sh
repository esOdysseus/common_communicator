#!/bin/bash
CUR_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
ROOT_PATH=${CUR_DIR}/..

# set custom variables
BUILD_MODE=release
TARGET=sample_tcp_client
DESP_PROTOCOL=${ROOT_PATH}/${BUILD_MODE}/config/desp_UniversalCMD_protocol.json
DESP_ALIAS=${ROOT_PATH}/${BUILD_MODE}/config/desp_alias.json

# set environment
GDB_BIN=gdb
GDB_FILE=${ROOT_PATH}/core
export LD_LIBRARY_PATH=${ROOT_PATH}/${BUILD_MODE}/lib:${LD_LIBRARY_PATH}

# for Valgrind debugging
VAL_BIN=valgrind
VAL_FILE=${ROOT_PATH}/${BUILD_MODE}/bin/valgrind_memcheck_${TARGET}.txt
VAL_OPT="-v --leak-check=full --error-limit=no --trace-children=yes --show-reachable=yes --track-origins=yes"
# VAL_OPT="-v --leak-check=full --error-limit=no --trace-children=yes --show-reachable=yes"
touch ${VAL_FILE}


cd ${ROOT_PATH}/${BUILD_MODE}/bin
# run program.
./${TARGET} ${DESP_ALIAS} ${DESP_PROTOCOL}   # for simple run program
#${GDB_BIN} ./${TARGET}  ${GDB_FILE}        # for gdb debug
#${VAL_BIN} ${VAL_OPT} ./${TARGET} ${DESP_ALIAS} ${DESP_PROTOCOL}  >& ${VAL_FILE} # for valgrind debug


