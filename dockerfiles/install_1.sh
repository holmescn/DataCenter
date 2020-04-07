#!/bin/sh
cd /tmp/vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install protobuf
export PATH=$PATH:/tmp/vcpkg/downloads/tools/cmake-3.14.0-linux/cmake-3.14.0-Linux-x86_64/bin
sed -i -E "s/(\s*install_service\s*)$/#\1/" /tmp/src/deps/TDengine/packaging/tools/make_install.sh
cd /tmp
mkdir build
cd build
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=/tmp/vcpkg/scripts/buildsystems/vcpkg.cmake /tmp/src
cmake --build .
