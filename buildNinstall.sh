#! /bin/bash

# for centos 7
# source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-centos7-gcc12-opt/setup.sh
# for alma9
source /cvmfs/sft.cern.ch/lcg/views/LCG_107/x86_64-el9-gcc14-opt/setup.sh


export INSTALL_DIR_PATH=$PWD/install

export PATH=$PATH:$INSTALL_DIR_PATH/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib

# Check if the build directory exists. If not, create and configure it.
if [ ! -d "install" ]; then
    echo "Install directory does not exist. Creating a fresh install directory."
    mkdir install
fi

if [ ! -d "build" ]; then
    echo "Build directory does not exist. Creating a fresh build directory."
    mkdir build
    cd build
    cmake ..
else
    # If the build directory exists, clean the previous build and re-configure.
    echo "Build directory already exists. Cleaning previous build."
    cd build
    make clean
    cmake ..
fi

make -j 4 install
cd ..
