#!/usr/bin/env bash

# This script will build the thirdparty libs required for the GPUdb API.
# - avrocpp 1.7.7
# - snappy  1.1.3
# 
# You must have cmake and a valid boost development library already installed.
#
# The environment variable AVRO_CMAKE_FLAGS can be used to specify an alternate
# boost root directory.
# Please see ../README.md for more information.

# The directory of this script.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# ---------------------------------------------------------------------------
# Run a command and append output to $LOG file which should have already been set.

function run_cmd
{
    local CMD=$1

    echo " "  >> $LOG
    echo $CMD 2>&1 | tee -a $LOG
    eval $CMD 2>&1 | tee -a $LOG

    #ret_code=$? this is return code of tee
    ret_code=${PIPESTATUS[0]}
    if [ $ret_code != 0 ]; then
        printf "Error : [%d] when executing command: '$CMD'\n" $ret_code
        echo "Please see log file: $LOG"
        exit 1
    fi
}

# ---------------------------------------------------------------------------

ROOT_DIR=$SCRIPT_DIR

BUILD_DIR=$ROOT_DIR/build
INSTALL_DIR=$ROOT_DIR/install

pushd $ROOT_DIR > /dev/null

mkdir -p $BUILD_DIR
mkdir -p $INSTALL_DIR

LOG=$ROOT_DIR/build/build.log

pushd $BUILD_DIR > /dev/null

# ----------------------------------------------------------------------------
# Build snappy

if [ ! -f $BUILD_DIR/snappy-1.1.3.built ]; then
    echo
    echo "Starting build of snappy..."

    # Clear remnants of a previous failed build.
    rm -Rf $BUILD_DIR/snappy-1.1.3

    run_cmd "tar -xzf ../snappy-1.1.3.tar.gz"
    pushd $BUILD_DIR/snappy-1.1.3 > /dev/null

    run_cmd "./configure --prefix=$INSTALL_DIR"
    run_cmd "make -j8"
    run_cmd "make install"

    date > $BUILD_DIR/snappy-1.1.3.built

    popd > /dev/null
    echo
fi

# ----------------------------------------------------------------------------
# Build Avro

if [ ! -f $BUILD_DIR/avro-cpp-1.7.7.built ]; then
    echo
    echo "Starting build of avro..."

    # Clear remnants of a previous failed build.
    rm -Rf $BUILD_DIR/avro-cpp-1.7.7

    run_cmd "tar -xzf ../avro-cpp-1.7.7.tar.gz"
    pushd $BUILD_DIR/avro-cpp-1.7.7

    # These macros are renamed in boot 1.59 (should be fixed in Avro 1.7.8)
    run_cmd "sed -i 's/BOOST_MESSAGE/BOOST_TEST_MESSAGE/g' test/buffertest.cc"
    run_cmd "sed -i 's/BOOST_CHECKPOINT/BOOST_TEST_CHECKPOINT/g' test/SchemaTests.cc"

    mkdir -p build
    pushd build > /dev/null

    run_cmd "cmake $GPUDB_AVRO_CMAKE_FLAGS -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_INSTALL_RPATH='$INSTALL_DIR/lib' ../"
    run_cmd "make -j8"
    run_cmd "make install"

    date > $BUILD_DIR/avro-cpp-1.7.7.built

    popd > /dev/null
    echo
fi

echo
echo "Success."
echo "Build   dir: $BUILD_DIR"
echo "Install dir: $INSTALL_DIR"
