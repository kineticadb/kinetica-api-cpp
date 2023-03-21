#!/usr/bin/env bash

# The directory of this script.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
THIS_SCRIPT=$(readlink -e "$0")

set -o nounset
set -o errexit

HELP_STR="
Usage: build-thirdparty-libs.sh [OPTIONS]

This script builds the third-party libs required for the gpudb-api-cpp.

Libraries that will be built:
    avrocpp 1.7.7 (requires boost)
    snappy  1.1.3
    boost   System, else use --boost-archive and supply source tgz.

You must have cmake and a valid boost development library installed to
build avro and the GPUdb api.
The boost development headers and libs can be usually be installed
with the operating system package manager.

On Debian/Ubuntu style OSes install:
 > sudo apt install chrpath cmake gcc g++ libboost-filesystem-dev libboost-iostreams-dev libboost-program-options-dev libboost-regex-dev libboost-system-dev libboost-thread-dev libssh-dev
On RHEL style OSes install:
 > sudo yum install chrpath cmake gcc gcc-c++ boost-filesystem boost-iostreams boost-program-options boost-regex boost-system boost-thread libssh-devel

The environment variable GPUDB_AVRO_CMAKE_FLAGS can be used to specify an alternate
boost root directory.

After running this script a '_build/build' and '_build/install' directory are
created in the root directory. 
The gpudb-cpp-api CMakeLists.txt GPUDB_API_CPP_LIBS_DIR variable is used to
set the -I compiler include and -L library path to this third-party 'install' directory.
If you change the default 'install' directory you must specify the path to the
install dir using the cmake flag below when building the GPUdb api.
'-DGPUDB_API_CPP_LIBS_DIR=/path/to/gpudb-api-cpp/thirdparty/install'


Please see ../README.md for more information.

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

ROOT_DIR="$SCRIPT_DIR"
GPUDB_API_CPP_DIR="$SCRIPT_DIR/.."
ROOT_BUILD_DIR="$GPUDB_API_CPP_DIR/_build"
BUILD_DIR="$ROOT_BUILD_DIR/thirdparty"
INSTALL_DIR="$ROOT_BUILD_DIR/thirdparty/install"
FORCE_REBUILD=0

LIBS_TO_BUILD="avro snappy" # "boost avro snappy" Note that ubuntu has snappy-devel, but rhel8 needs epel for it.

AVRO_ARCHIVE="${AVRO_ARCHIVE:-$ROOT_DIR/avro-cpp-1.7.7.tar.gz}"
AVRO_ARCHIVE=$(readlink -e "$AVRO_ARCHIVE") || exit 1
AVRO_ARCHIVE_NAME=$(basename "$AVRO_ARCHIVE" | sed 's/\.tar\..*$//g') # 'avro-cpp-1.7.7'

SNAPPY_ARCHIVE="${SNAPPY_ARCHIVE:-$SCRIPT_DIR/snappy-1.1.3.tar.gz}"
SNAPPY_ARCHIVE=$(readlink -e "$SNAPPY_ARCHIVE") || exit 1
SNAPPY_ARCHIVE_NAME=$(basename "$SNAPPY_ARCHIVE" | sed 's/\.tar\..*$//g') # 'snappy-1.1.3'

BOOST_ARCHIVE=$(ls "$SCRIPT_DIR/boost_"* 2>/dev/null | grep -E '\.tgz$|\.tar.gz$|\.tar.bz2$' | tail -n 1)

GPUDB_AVRO_CMAKE_FLAGS="${GPUDB_AVRO_CMAKE_FLAGS:-}"

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
        --libs)
            LIBS_TO_BUILD="$1"
            shift
            ;;
        --boost-archive)
            BOOST_ARCHIVE=$(readlink -m "$1")
            LIBS_TO_BUILD+=" boost"
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
    local SNAPPY_BUILD_DIR="$BUILD_DIR/$SNAPPY_ARCHIVE_NAME"

    if [ $FORCE_REBUILD -eq 1 ] || [ ! -f "$BUILD_DIR/$SNAPPY_ARCHIVE_NAME.built" ]; then
        echo | tee -a "$LOG"
        echo "Starting build of snappy..." | tee -a "$LOG"

        # Clear remnants of a previous failed build.
        rm -Rf "$BUILD_DIR/$SNAPPY_ARCHIVE_NAME"
        rm -Rf "$INSTALL_DIR"/include/snappy*.h

        run_cmd "tar -xf '$SNAPPY_ARCHIVE'"
        pushd "$SNAPPY_BUILD_DIR" > /dev/null

        local BUILD_CXXFLAGS=""
        if [ "${STATIC_WITH_PIC}" -eq 1 ]; then
            BUILD_CXXFLAGS="-fPIC"
        fi
        echo "DISABLE_CXX11_ABI=${DISABLE_CXX11_ABI}"
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
            run_cmd "make -j '$NUM_PROCESSORS' install"
        popd > /dev/null
    fi
}

# ----------------------------------------------------------------------------
# Build Avro

function build_avro
{
    local AVRO_BUILD_DIR="$BUILD_DIR/$AVRO_ARCHIVE_NAME/build"

    if [ ! -f "$BUILD_DIR/$AVRO_ARCHIVE_NAME.built" ]; then
        echo
        echo "Starting build of avro..." | tee -a "$LOG"

        # Clear remnants of a previous failed build.
        rm -Rf "$BUILD_DIR/$AVRO_ARCHIVE_NAME"
        rm -Rf "$INSTALL_DIR"/include/avro/*

        run_cmd "tar -xf '$AVRO_ARCHIVE'"

        pushd "$BUILD_DIR/$AVRO_ARCHIVE_NAME" || exit 1

        run_cmd "sed -i '1s;^;#pragma GCC system_header\n#pragma GCC diagnostic push\n#pragma GCC diagnostic ignored \"-Wdeprecated-declarations\"\n;' api/DataFile.hh api/Stream.hh"
        #printf "\n%s\n" "#pragma GCC diagnostic pop" >> api/DataFile.hh
        #printf "\n%s\n" "#pragma GCC diagnostic pop" >> api/Stream.hh

        # These macros are renamed in boost 1.59 (should be fixed in Avro 1.7.8)
        run_cmd "sed -i 's/BOOST_MESSAGE/BOOST_TEST_MESSAGE/g' test/buffertest.cc"
        run_cmd "sed -i 's/BOOST_CHECKPOINT/BOOST_TEST_CHECKPOINT/g' test/SchemaTests.cc"

        run_cmd "mkdir -p '$AVRO_BUILD_DIR'"
        pushd "$AVRO_BUILD_DIR" > /dev/null || exit 1

        if [ "${STATIC_WITH_PIC}" -eq 1 ]; then
            GPUDB_AVRO_CMAKE_FLAGS+=" -DCMAKE_CXX_FLAGS_RELEASE:STRING='-fPIC'"
        fi

        # Point to the local install/ that was just built.
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

        run_cmd "date > '$BUILD_DIR/$AVRO_ARCHIVE_NAME.built'"

        popd > /dev/null
    else
        echo  | tee -a "$LOG"
        echo "Skipping $AVRO_ARCHIVE_NAME build, already built, installing from $AVRO_BUILD_DIR" | tee -a "$LOG"
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
    if [ -z "$BOOST_ARCHIVE" ]; then
        echo "ERROR: Please specify the path to a boost archive using '--boost-archive' or by placing a boost archive in the $SCRIPT_DIR folder." | tee -a "$LOG"
        echo "       Boost archives can be downloaded from: http://www.boost.org" | tee -a "$LOG"
        exit 1
    fi

    if [ ! -f "$BOOST_ARCHIVE" ]; then
        echo "ERROR: No Boost archive at '$BOOST_ARCHIVE'. Please specify the path to a boost archive using '--boost-archive' or place a boost archive in the $SCRIPT_DIR folder." | tee -a "$LOG"
        echo "       Boost archives can be downloaded from: http://www.boost.org" | tee -a "$LOG"
        exit 1
    fi

    echo "Using Boost library at $BOOST_ARCHIVE." | tee -a "$LOG"

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
AVRO_ARCHIVE=$AVRO_ARCHIVE
BOOST_ARCHIVE=$BOOST_ARCHIVE
SNAPPY_ARCHIVE=$SNAPPY_ARCHIVE
DISABLE_CXX11_ABI=$DISABLE_CXX11_ABI
STATIC_WITH_PIC=$STATIC_WITH_PIC
COMPILER=${CC:-gcc} $(${CC:-gcc} -dumpfullversion -dumpversion)"
}


# ---------------------------------------------------------------------------
# Main script

mkdir -p "$BUILD_DIR" || exit 1

LOG="$BUILD_DIR/build.log"

run_cmd "which cmake && cmake --version"
run_cmd "which chrpath && chrpath --version"

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

        [[ "$LIBS_TO_BUILD" == *boost*  ]] && build_boost
        [[ "$LIBS_TO_BUILD" == *snappy* ]] && build_snappy
        [[ "$LIBS_TO_BUILD" == *avro*   ]] && build_avro

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
