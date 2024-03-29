#!/bin/bash

set -e

mkdir -p lib
pushd lib

curl -L -o protobuf-cpp-3.21.2.tar.gz -C - https://github.com/protocolbuffers/protobuf/releases/download/v21.2/protobuf-cpp-3.21.2.tar.gz

for x in *.tar.gz; do tar xf $x; done

pushd protobuf-3.21.2/
./configure --prefix=`pwd`/dist && make && make install
popd

popd
