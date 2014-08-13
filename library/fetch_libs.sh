#!/bin/bash
DIR="$(dirname $0)"

cd ./third_party
git submodule update --init --recursive

# configure libevhtp
cd ./libevhtp
git pull origin master 
git checkout master
cd ./build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=$DIR/../../third_party -DEVHTP_DISABLE_REGEX=ON
cd ../../

# configure ctemplate
cd ./ctemplate
git pull origin master 
git checkout master
DIR="$(readlink -m "$(pwd)")"
echo $DIR
./configure --prefix=$DIR/..
cd ../

# configure ctemplate
cd ./ctemplate
cd ../

# build libevhtp
cd ./libevhtp/build
make
make install
cd ../../


# build ctemplate
cd ./ctemplate
make
make install
cd ..
