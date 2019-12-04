#!/bin/bash

rm -rf ./debug
rm -rf ./api/libcommunicator.so*

if [ "${1}" != "clear" ]
then
    mkdir -p ./debug
    cd ./debug
    qmake ..
    make
else
    rm -rf ./example/debug
fi

