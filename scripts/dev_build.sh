#!/bin/sh
cd /tmp
git clone --depth=1 -b release/v1.6.5.6 --single-branch https://github.com/taosdata/TDengine.git
cd TDengine
sed -i -E "s/(\s*install_service\s*)$/#\1/" packaging/tools/make_install.sh
mkdir build
cd build
cmake ..
cmake --build .
make install

cd /tmp
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local /tmp/src
cmake --build .
make install
