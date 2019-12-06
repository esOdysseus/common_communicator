#!/bin/bash

BUILD_MODE=${1}
if [ -z "${1}" ]
then
    BUILD_MODE=release
fi
INSTALL_FOLDER=${PWD}/${BUILD_MODE}

echo
echo "BUILD_MODE=${BUILD_MODE}"
echo "INSTALL_FOLDER=${INSTALL_FOLDER}"
echo

if [ "${2}" == "clean" ];then
    echo ">>>> Clear all-data of installation & objects."
    make clean
    cd ./example
    make clean
    rm -rf ./Makefile
    rm -rf ./common_api/Makefile
    cd ../
    rm -rf ./debug
    rm -rf ./release
    rm -rf ./pkgconfig
    rm -rf ./Makefile
elif [ "${2}" == "example" ];then
    echo ">>>> Build example & install"
    cd ./example
    qmake BUILD_MODE=${BUILD_MODE} DESTDIR=${INSTALL_FOLDER}/bin
    make
    make install
else
    echo ">>>> Build communicator-sdk & install"
    qmake BUILD_MODE=${BUILD_MODE}
    make
    make install
fi

