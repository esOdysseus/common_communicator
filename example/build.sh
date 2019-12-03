#!/bin/bash

rm -rf ./debug
mkdir -p ./debug
cd ./debug
qmake ..
make

