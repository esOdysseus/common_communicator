#!/bin/bash
__CUR_DIR__=${PWD}
ROOT_PATH_SCRIPT_FILE=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
CPU_ARCH=${1}
BOARD_TARGET=${2}

# Set Environment.
case ${CPU_ARCH} in
    "armv7" )
        echo "Set Environment for armv7 ARCH."
        echo

        # Set variables according to BOARD_TARGET.
        case ${BOARD_TARGET} in
            "arndale5250" )     # checking ...
                TOOLCHAIN_PATH="/usr/local/gcc-linaro/arm-linux-gnueabi/4.9.4_x86_64"
                # TOOLCHAIN_PATH="/usr/local/gcc-linaro/arm-linux-gnueabihf/4.7-2013.03_x86_32"
                CROSS_COMPILER_PREFIX="arm-linux-gnueabi-"
                ;;
            "orangepi-i96" )    # valid tool-chain path !!
                TOOLCHAIN_PATH="/usr/local/gcc-linaro/arm-linux-gnueabihf/4.9.4_x86_64"
                CROSS_COMPILER_PREFIX="arm-linux-gnueabihf-"
                ;;
            * ) 
                echo -e "\e[1;31m [ERROR] Not Supported BOARD_TARGET.(${BOARD_TARGET}) \e[0m"
                ;;
        esac

        # Set Cross-Compiler Variables & Environment.
        export CC="${CROSS_COMPILER_PREFIX}gcc"
        export CXX="${CROSS_COMPILER_PREFIX}g++"
        export LINK="${CROSS_COMPILER_PREFIX}g++"
        export PATH=${TOOLCHAIN_PATH}/bin:${PATH}
        # export PKG_CONFIG_PATH=""
        # export CUSTOM_MODULE_DIR=""
        ;;
    * )
        echo -e "\e[1;31m [ERROR] Not Supported CPU_ARCH.(${CPU_ARCH}) \e[0m"
        exit 0
        ;;
esac


# Exit this script
cd ${__CUR_DIR__}
