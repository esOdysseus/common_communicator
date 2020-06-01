#!/bin/bash
ROOT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# set custom variables
BUILD_MODE=debug
TARGET=sample_uds_udp_server
DESP_PROTOCOL=${ROOT_PATH}/${BUILD_MODE}/config/desp_protocol.json
DESP_ALIAS=${ROOT_PATH}/${BUILD_MODE}/config/desp_alias.json

# set environment
GDB_BIN=gdb
GDB_FILE=${ROOT_PATH}/core
export LD_LIBRARY_PATH=${ROOT_PATH}/${BUILD_MODE}/lib:${LD_LIBRARY_PATH}

# for Valgrind debugging
VAL_BIN=valgrind
VAL_FILE=${ROOT_PATH}/${BUILD_MODE}/bin/valgrind_memcheck_${TARGET}.txt
VAL_OPT="-v --leak-check=full --error-limit=no --trace-children=yes --show-reachable=yes"
touch ${VAL_FILE}


cd ${ROOT_PATH}/${BUILD_MODE}/bin
# run program.
./${TARGET} "192.168.1.2" "12346" ${DESP_PROTOCOL} ${DESP_ALIAS}  # for simple run program
#${GDB_BIN} ./${TARGET}  ${GDB_FILE}        # for gdb debug
#${VAL_BIN} ${VAL_OPT} ./${TARGET} "192.168.1.2" "12346" ${DESP_PROTOCOL} ${DESP_ALIAS} >& ${VAL_FILE} # for valgrind debug


