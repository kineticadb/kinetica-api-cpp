#!/usr/bin/env bash

# The directory of this script.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
THIS_SCRIPT=$(readlink -e "$0")

HELP_STR="
Usage: build-thirdparty-libs.sh [OPTIONS]

This script builds the third-party libs required for the gpudb-api-cpp.
You must have cmake and a valid boost development library already installed.

A 'build' and 'install' directory are created and the gpudb-app-api will
set the -I compiler include and -L library path to the the this third-party
'install' directory.

If you change the default 'install' directory you must specify the path to the
install dir using the cmake flag below when building the GPUdb api.
'-DGPUDB_API_CPP_LIBS_DIR=/path/to/gpudb-api-cpp/thirdparty/install'

The environment variable AVRO_CMAKE_FLAGS can be used to specify an alternate
boost root directory.
Please see ../README.md for more information.

Libraries that will be built:
    avrocpp 1.7.7
    snappy  1.1.3

Options:
 -b --build-dir       : Directory to put temp build files (default ./build)
 -i --install-dir     : Directory to install dist to (default ./install)
                        It is best to not use a system path, prefer a local one.
 -r --rebuild         : Force rebuild, no check for existing dist file.
 -h --help            : Print this help.
"

# ---------------------------------------------------------------------------
# Run a command and append output to $LOG file which should have already been set.

function run_cmd
{
    local CMD=$1

    echo " "  >> "$LOG"
    echo "$CMD" 2>&1 | tee -a "$LOG"
    eval "$CMD" 2>&1 | tee -a "$LOG"

    #ret_code=$? this is return code of tee
    ret_code=${PIPESTATUS[0]}
    if [ $ret_code != 0 ]; then
        printf "Error : [%d] when executing command: '$CMD'\n" $ret_code
        echo "Please see log file: $LOG"
        exit 1
    fi
}

# ---------------------------------------------------------------------------
# Parse command line options
# ---------------------------------------------------------------------------

ROOT_DIR=$SCRIPT_DIR
GPUDB_API_CPP_DIR=$SCRIPT_DIR/..
ROOT_BUILD_DIR=$GPUDB_API_CPP_DIR/_build
BUILD_DIR=$ROOT_BUILD_DIR/thirdparty
INSTALL_DIR=$ROOT_BUILD_DIR/thirdparty/install
FORCE_REBUILD=0

BOOST_ARCHIVE="$(ls $SCRIPT_DIR/boost_* 2>/dev/null | grep -E '\.tgz$|\.tar.gz$|\.tar.bz2$' | tail -n 1)"

DISABLE_CXX11_ABI=0
STATIC_WITH_PIC=0

NUM_PROCESSORS=$(nproc)

# grab command line args
while [[ $# > 0 ]]; do
    key="$1"
    shift

    case $key in
        -b|--build-dir)
            BUILD_DIR=$(readlink -m "$1")
            shift
            ;;
        -i|--install-dir)
            INSTALL_DIR=$(readlink -m "$1")
            shift
            ;;
        --boost-archive)
            BOOST_ARCHIVE=$(readlink -m "$1")
            shift
            ;;
        --disable-cxx11-abi)
            DISABLE_CXX11_ABI=1
            ;;
        --static-with-pic)
            STATIC_WITH_PIC=1
            ;;
        -r|--rebuild)
            FORCE_REBUILD=1
            ;;
        -h|--help)
            echo "$HELP_STR"
            exit 0
            ;;
        *)
            echo "Unknown option: '$key', exiting."
            exit 1
            ;;
    esac
done

# ----------------------------------------------------------------------------
# Build snappy

function build_snappy
{
    local SNAPPY_BUILD_DIR="$BUILD_DIR/snappy-1.1.3"

    if [ $FORCE_REBUILD -eq 1 ] || [ ! -f "$BUILD_DIR/snappy-1.1.3.built" ]; then
        echo | tee -a "$LOG"
        echo "Starting build of snappy..." | tee -a "$LOG"

        # Clear remnants of a previous failed build.
        rm -Rf $BUILD_DIR/snappy-1.1.3*
        rm -Rf $INSTALL_DIR/include/snappy*.h

        run_cmd "tar -xzf $ROOT_DIR/snappy-1.1.3.tar.gz"
        pushd "$SNAPPY_BUILD_DIR" > /dev/null

        local BUILD_CXXFLAGS=""
        if [ "${STATIC_WITH_PIC}" -eq 1 ]; then
            BUILD_CXXFLAGS="-fPIC"
        fi
        echo "DISABLE_CXX_ABI=${DISABLE_CXX_ABI}"
        if [ "${DISABLE_CXX11_ABI}" -eq "1" ]; then
            BUILD_CXXFLAGS="$BUILD_CXXFLAGS -D_GLIBCXX_USE_CXX11_ABI=0 "
        fi

        if [ -n "${BUILD_CXXFLAGS}" ]; then
            run_cmd "CXXFLAGS=\"${BUILD_CXXFLAGS}\" ./configure '--prefix=$INSTALL_DIR'"
        else
            run_cmd "./configure --prefix=$INSTALL_DIR"
        fi
        run_cmd "make -j '$NUM_PROCESSORS' install"

        run_cmd "date > $BUILD_DIR/snappy-1.1.3.built"

        popd > /dev/null
        echo
    else
        echo | tee -a "$LOG"
        echo "Skipping snappy-1.1.3 build, already built, installing from $SNAPPY_BUILD_DIR" | tee -a "$LOG"
        pushd "$SNAPPY_BUILD_DIR" > /dev/null || exit 1
            run_cmd "make -j8 install"
        popd > /dev/null
    fi
}

# ----------------------------------------------------------------------------
# Build Avro

function build_avro
{
    local AVRO_BUILD_DIR="$BUILD_DIR/avro-cpp-1.7.7/build"

    if [ ! -f $BUILD_DIR/avro-cpp-1.7.7.built ]; then
        echo
        echo "Starting build of avro..." | tee -a "$LOG"

        # Clear remnants of a previous failed build.
        rm -Rf $BUILD_DIR/avro-cpp-1.7.7*
        rm -Rf $INSTALL_DIR/include/avro/*

        run_cmd "tar -xzf $ROOT_DIR/avro-cpp-1.7.7.tar.gz"

        pushd "$BUILD_DIR/avro-cpp-1.7.7" || exit 1

        # These macros are renamed in boost 1.59 (should be fixed in Avro 1.7.8)
        run_cmd "sed -i 's/BOOST_MESSAGE/BOOST_TEST_MESSAGE/g' test/buffertest.cc"
        run_cmd "sed -i 's/BOOST_CHECKPOINT/BOOST_TEST_CHECKPOINT/g' test/SchemaTests.cc"

        run_cmd "mkdir -p '$AVRO_BUILD_DIR'"
        pushd "$AVRO_BUILD_DIR" > /dev/null || exit 1

        local GPUDB_AVRO_CMAKE_FLAGS=""

        if [ "${STATIC_WITH_PIC}" -eq 1 ]; then
            GPUDB_AVRO_CMAKE_FLAGS="$GPUDB_AVRO_CMAKE_FLAGS -DCMAKE_CXX_FLAGS_RELEASE:STRING='-fPIC'"
        fi

        if [ -d "$INSTALL_DIR/include/boost" ]; then
            GPUDB_AVRO_CMAKE_FLAGS="-DBoost_NO_BOOST_CMAKE=TRUE -DBOOST_INCLUDEDIR=/$INSTALL_DIR/include -DBOOST_LIBRARYDIR=$INSTALL_DIR/lib $GPUDB_AVRO_CMAKE_FLAGS"
        fi

        run_cmd "cmake $GPUDB_AVRO_CMAKE_FLAGS -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -DCMAKE_INSTALL_RPATH='$INSTALL_DIR/lib' ../"

        # Unfortunately, it seems the only way to get this flag in there is to manually
        # update the makefiles after cmake has generated them.
        if [ "$DISABLE_CXX11_ABI" -eq "1" ] || [ "${STATIC_WITH_PIC}" -eq "1" ]; then
            local ABI_FLAGS=""
            local PIC_FLAGS=""
            if [ "${DISABLE_CXX11_ABI}" -eq "1" ]; then
                ABI_FLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
            fi
            if [ "${STATIC_WITH_PIC}" -eq "1" ]; then
                PIC_FLAGS="-fPIC"
            fi
            for f in $(find . -name "flags.make"); do
                run_cmd "sed -i 's/CXX_FLAGS = /CXX_FLAGS = ${PIC_FLAGS} ${ABI_FLAGS} /g' $f"
            done
            run_cmd "sed -i 's/CMAKE_CXX_FLAGS:STRING=/CMAKE_CXX_FLAGS:STRING=${PIC_FLAGS} ${ABI_FLAGS} /g' $f"
        fi

        run_cmd "make -j '$NUM_PROCESSORS' install"

        run_cmd "date > $BUILD_DIR/avro-cpp-1.7.7.built"

        popd > /dev/null
    else
        echo  | tee -a "$LOG"
        echo "Skipping avro 1.7.7 build, already built, installing from $AVRO_BUILD_DIR" | tee -a "$LOG"
        pushd "$AVRO_BUILD_DIR" || exit 1
            run_cmd "make -j '$NUM_PROCESSORS' install"
        popd > /dev/null
    fi
    echo
}

# ----------------------------------------------------------------------------
# Build boost - largely stolen from build-gpudb-core-libs.sh

function build_boost
{
    local BOOST_NAME=$(echo $(basename "$BOOST_ARCHIVE") | awk -F'.' '{print $1}')

    local BJAM_FLAGS=""
    if [ $DISABLE_CXX11_ABI -eq 1 ]; then
        BJAM_FLAGS="$BJAM_FLAGS define=_GLIBCXX_USE_CXX11_ABI=0"
    fi
    if [ "$STATIC_WITH_PIC" -eq 1 ]; then
        BJAM_FLAGS="$BJAM_FLAGS cxxflags='-fPIC $CXXFLAGS'"
    fi

    if [ ! -f "$BUILD_DIR/$BOOST_NAME.built" ]; then
        pushd "$BUILD_DIR" > /dev/null
            echo
            echo "Starting build of boost..."

            # Clear remnants of a previous failed build.
            run_cmd "rm -Rf '$BUILD_DIR/$BOOST_NAME'"

            run_cmd "tar -xf '$BOOST_ARCHIVE'"
            pushd "$BUILD_DIR/$BOOST_NAME" > /dev/null || exit 1
                MACHINE_NAME="$(uname -m)"
                # Check if we're running on powerpc.
                if [[ "a$MACHINE_NAME" == "appc64le" ]]; then
                    IS_PPC64LE=1
                else
                    IS_PPC64LE=0
                fi

                if [[ "$IS_PPC64LE" == "1" ]]; then
                    # This func is macro in ppc64le, https://bugzilla.redhat.com/show_bug.cgi?id=1262444
                    run_cmd "sed -i 's/::feclearexcept/feclearexcept/g' ./boost/test/impl/execution_monitor.ipp"

                    # Fix ppc/gcc5.4/boost 1.65 "unresolvable R_PPC64_ADDR64 against `rand@@GLIBC_2.17'" link error using uuid generator
                    # https://bugzilla.suse.com/show_bug.cgi?id=955832
                    run_cmd "sed -i 's@^[ \t]*sha.process_bytes( (unsigned char const\*)&std::rand@  // (disabled for ppc) sha.process_bytes( (unsigned char const*)&std::rand@g' boost/uuid/seed_rng.hpp"
                    run_cmd "grep '// (disabled for ppc) sha.process_bytes' boost/uuid/seed_rng.hpp"
                fi

                # system and thread for us
                # system, filesystem and program_options for avro
                #   (see https://avro.apache.org/docs/1.7.7/api/cpp/html/index.html)
                # iostreams for filesystem
                # chrono, datetime and regex for Simba.
                echo "using gcc : : : <linkflags>-Wl,-rpath,'\${ORIGIN}',-rpath,$DUMMY_RPATH ; " > user-config.jam
                run_cmd "./bootstrap.sh --prefix=$INSTALL_DIR --with-libraries=system,thread,iostreams,filesystem,program_options,chrono,date_time,regex --without-icu"

                # Find the path to bjam
                local BJAM_EXE=$(ls -1 "$PWD/bjam" "$PWD/tools/build/src/engine/bjam" 2> /dev/null | head -n 1)
                if [ ! -f "${BJAM_EXE}" ]; then
                    echo "ERROR: Could not find bjam:'${BJAM_EXE}' to build boost!"
                    exit 1
                fi

                run_cmd "$BJAM_EXE -a $BJAM_FLAGS --layout=system install variant=release threading=multi --user-config=user-config.jam --disable-icu boost.locale.icu=off '--prefix=$INSTALL_DIR' '-j$NUM_PROCESSORS' install"

                date > "$BUILD_DIR/$BOOST_NAME.built"
            popd > /dev/null
            echo
        popd > /dev/null
    else
        echo  | tee -a "$LOG"
        echo "Skipping boost build, already built, installing from $BUILD_DIR/$BOOST_NAME" | tee -a "$LOG"
        pushd "$BUILD_DIR/$BOOST_NAME" > /dev/null || exit 1
            local BJAM_EXE=$(ls -1 "$PWD/bjam" "$PWD/tools/build/src/engine/bjam" 2> /dev/null | head -n 1)
            if [ ! -f "${BJAM_EXE}" ]; then
                echo "ERROR: Could not find bjam:'${BJAM_EXE}' to build boost!"
                exit 1
            fi

            run_cmd "$BJAM_EXE -a $BJAM_FLAGS --layout=system install variant=release threading=multi --user-config=user-config.jam --disable-icu boost.locale.icu=off '--prefix=$INSTALL_DIR' '-j$NUM_PROCESSORS' install"
        popd > /dev/null
    fi

    sed -i 's@// signbit@// signbit\n#undef signbit // clear #define signbit from /usr/include/math.h@g' "$INSTALL_DIR/include/boost/math/special_functions/sign.hpp"
    run_cmd "grep -F '// clear #define signbit from /usr/include/math.h' '$INSTALL_DIR/include/boost/math/special_functions/sign.hpp'"
}

function get_build_info
{
    echo "$(basename "$THIS_SCRIPT")=$(md5sum "$THIS_SCRIPT")
BUILD_DIR=$BUILD_DIR
INSTALL_DIR=$INSTALL_DIR
BOOST_ARCHIVE=$BOOST_ARCHIVE
DISABLE_CXX11_ABI=$DISABLE_CXX11_ABI
STATIC_WITH_PIC=$STATIC_WITH_PIC
COMPILER=${CC:-gcc} $(${CC:-gcc} -dumpfullversion -dumpversion)"
}


# ---------------------------------------------------------------------------
# Main script

mkdir -p "$BUILD_DIR" || exit 1

LOG="$BUILD_DIR/build.log"

if [ -z "$BOOST_ARCHIVE" ]; then
    echo "ERROR: Please specify the path to a boost archive using '--boost-archive' or by placing a boost archive in the $SCRIPT_DIR folder.  Boost archives can be downloaded from: http://www.boost.org" | tee -a "$LOG"
    exit 1
fi

if [ ! -f "$BOOST_ARCHIVE" ]; then
    echo "ERROR: No Boost archive at '$BOOST_ARCHIVE'.  Please specify the path to a boost archive using '--boost-archive' or place a boost archive in the $SCRIPT_DIR folder.  Boost archives can be downloaded from: http://www.boost.org" | tee -a "$LOG"
    exit 1
fi

echo "Using Boost library at $BOOST_ARCHIVE." | tee -a "$LOG"

pushd "$ROOT_DIR" > /dev/null

    run_cmd "mkdir -p '$INSTALL_DIR'"
    run_cmd "mkdir -p '$BUILD_DIR'"

    get_build_info > "$BUILD_DIR/BUILD_INFO_NEW"
    if [ ! -f "$BUILD_DIR/BUILD_INFO" ] || ! diff "$BUILD_DIR/BUILD_INFO" "$BUILD_DIR/BUILD_INFO_NEW"; then
        echo "Build environment has changed, rebuilding all thirdparty libs."
        rm -Rf "$BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi

    pushd "$BUILD_DIR" > /dev/null
        DUMMY_RPATH=0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
        export LDFLAGS="-Wl,-rpath,'\$\${ORIGIN}',-rpath,$DUMMY_RPATH"

        build_boost
        build_snappy
        build_avro

    popd > /dev/null

    pushd "$INSTALL_DIR/lib" > /dev/null
        run_cmd "chrpath -r \\\${ORIGIN} *.so"
    popd > /dev/null
popd > /dev/null

get_build_info > "$BUILD_DIR/BUILD_INFO"

echo
echo "Success."
echo "Build   dir: $BUILD_DIR"
echo "Install dir: $INSTALL_DIR"
