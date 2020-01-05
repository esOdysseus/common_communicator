#!/bin/bash
ROOT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
export ROOT_PATH=${ROOT_PATH}

BUILD_MODE=${1}
if [ -z "${1}" ]
then
    BUILD_MODE=release
fi
BUILD_DIR=${ROOT_PATH}/build
INSTALL_DIR=${ROOT_PATH}/${BUILD_MODE}

echo
echo "BUILD_MODE=${BUILD_MODE}"
echo "BUILD_DIR=${BUILD_DIR}"
echo "INSTALL_DIR=${INSTALL_DIR}"
echo

# enter build-situation
if [ "${2}" == "clean" ];then
    if [ ! -d "${BUILD_DIR}" ]; then
        echo -e "\e[1;31m Can't clean. because Not exist BUILD_DIR. \e[0m"
        exit 0
    fi

    echo ">>>> Clear all-data of installation & objects. <<<<"
    rm -rf ${BUILD_DIR}
    rm -rf ${INSTALL_DIR}
else
    if [ ! -d "${BUILD_DIR}" ]; then
        mkdir -p ${BUILD_DIR}
    fi
    cd ${BUILD_DIR}

    if [ "${2}" == "example" ];then
        echo ">>>> Build example & install"
        qmake ${ROOT_PATH} TARGET=example BUILD_MODE=${BUILD_MODE} DESTDIR=${INSTALL_DIR}/bin
    elif [ "${2}" == "protocol" ];then
        echo ">>>> Build protocols-lib & install"
        rm -rf ${INSTALL_DIR}/lib/libproto*
        qmake ${ROOT_PATH} TARGET=protocol BUILD_MODE=${BUILD_MODE} DESTDIR=${INSTALL_DIR}/lib
    else
        echo ">>>> Build communicator-sdk & install"
        qmake ${ROOT_PATH} TARGET=communicator BUILD_MODE=${BUILD_MODE} DESTDIR=${INSTALL_DIR}/lib
    fi
    
    make
    make install
fi


# exit build-situation.
cd ${ROOT_PATH}
