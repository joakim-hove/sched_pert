#!/bin/bash

mkdir -p opm
pushd opm
if [ ! -d opm-common ]; then
   git clone https://github.com/opm/opm-common
fi

pushd opm-common
git fetch origin
git merge origin/master
popd

mkdir -p build

pushd build
cmake ../opm-common -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install
make -j 4
make install
popd

popd