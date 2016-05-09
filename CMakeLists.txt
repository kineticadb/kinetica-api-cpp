# ---------------------------------------------------------------------------
# GPUdb API CPP CMake build file
# ---------------------------------------------------------------------------

# only allow out of tree builds so we don't overwrite the gmake Makefile.

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" _in_tree_build)
if (_in_tree_build)
    message(SEND_ERROR "
 Building in the source dir is not allowed!
 Create a new directory then run '$cmake ${CMAKE_SOURCE_DIR}' from that new directory.
 You should delete the newly created 'CMakeCache.txt' and 'CMakeFiles/' dir
   in '${CMAKE_SOURCE_DIR}' first.")

    message(FATAL_ERROR "In-tree builds not allowed!")
endif (_in_tree_build)

# ---------------------------------------------------------------------------
# Project name, sets ${GPUDB_API_CPP_SOURCE_DIR} and ${GPUDB_API_CPP_BINARY_DIR}

project( GPUDB_API_CPP )

set(GPUDB_API_CPP_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH
    "Root dir of GPUdb API CPP" FORCE)

# ===========================================================================
# General settings for CMake
# ===========================================================================

# ---------------------------------------------------------------------------
# Use a minimum version of CMake of 2.8, >= 2.8.3 is prefered

cmake_minimum_required( VERSION 2.8 )

# ---------------------------------------------------------------------------
# Setup the CMake environment

set(CMAKE_BUILD_TYPE_DEFAULT RelWithDebInfo)
#set(BUILD_SHARED_LIBS        FALSE)

include( build/CMakeProject.cmake )
include( build/CMakeFunctions.cmake )

message(STATUS "* ---------------------------------------------------------------------------" )
message(STATUS "* GPUdb API CPP CMake command line options" )
message(STATUS "* " )
message(STATUS "* -DGPUDB_API_CPP_LIBS_DIR=${GPUDB_API_CPP_SOURCE_DIR}/thirdparty/install" )
message(STATUS "*   Specify the location of the 3rd party avro and snappy development libraries." )
message(STATUS "*   Please build them first using the thirdparty/build-thrirdparty-libs.sh script first." )
message(STATUS "*   The install/ dir should contain 'include/' and 'lib/' subdirectories." )
message(STATUS "* " )
message(STATUS "* ---------------------------------------------------------------------------" )
message(STATUS " " )

EXIT_IF_HELP_REQUESTED()

# ===========================================================================
# GPUDB_API_CPP version
# ===========================================================================

EXECUTE_PROCESS(COMMAND "date" OUTPUT_VARIABLE GPUDB_API_CPP_CMAKE_BUILD_DATE_STRING)
string(STRIP "${GPUDB_API_CPP_CMAKE_BUILD_DATE_STRING}" GPUDB_API_CPP_CMAKE_BUILD_DATE_STRING)

# Get the version info from the C++ API version file
file(READ "${CMAKE_SOURCE_DIR}/gpudb/VERSION" API_CPP_VERSION_STR)

string(REGEX MATCH "MAJOR[ \t]*=[ \t]*([0-9]+)"    _dummy ${API_CPP_VERSION_STR} )
set(API_CPP_VERSION_MAJOR   ${CMAKE_MATCH_1} CACHE INTERNAL "" FORCE)

string(REGEX MATCH "MINOR[ \t]*=[ \t]*([0-9]+)"    _dummy ${API_CPP_VERSION_STR} )
set(API_CPP_VERSION_MINOR   ${CMAKE_MATCH_1} CACHE INTERNAL "" FORCE)

string(REGEX MATCH "REVISION[ \t]*=[ \t]*([0-9]+)" _dummy ${API_CPP_VERSION_STR} )
set(API_CPP_VERSION_REVISION ${CMAKE_MATCH_1} CACHE INTERNAL "" FORCE)

string(REGEX MATCH "ABI_VERSION[ \t]*=[ \t]*([0-9]+)" _dummy ${API_CPP_VERSION_STR} )
set(API_CPP_ABI_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "" FORCE)


set( GPUDB_API_CPP_VERSION
    "${API_CPP_VERSION_MAJOR}.${API_CPP_VERSION_MINOR}.${API_CPP_VERSION_REVISION}.${API_CPP_ABI_VERSION}"
     CACHE INTERNAL "" FORCE)


include("build/CMakeGitVersion.cmake")
GetGitRepoVersionInfo(${CMAKE_SOURCE_DIR} GPUDB_API_CPP_)

set(GPUDB_API_CPP_GIT_HASH_STRING ${GPUDB_API_CPP_GIT_REPO_HASH_STRING})
set(GPUDB_API_CPP_GIT_DATE_STRING ${GPUDB_API_CPP_GIT_REPO_DATE_STRING})
if (GPUDB_API_CPP_GIT_REPO_MODIFIED)
    set(GPUDB_API_CPP_GIT_HASH_STRING "${GPUDB_API_CPP_GIT_HASH_STRING}*")
endif()

# ===========================================================================
# Add general targets
# ===========================================================================

enable_testing()
ADD_CPPCHECK_TEST(run_cppcheck)

set( DOXYGEN_PROJECT_NAME        "GPUdb &nbsp; C++ &nbsp; API" )
set( DOXYGEN_PROJECT_NUMBER      "Version &nbsp; ${GPUDB_API_CPP_VERSION}" )
set( DOXYGEN_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/doxygen" )
set( DOXYGEN_STRIP_FROM_PATH     "${GPUDB_API_CPP_ROOT_DIR}" )
set( DOXYGEN_STRIP_FROM_INC_PATH "${GPUDB_API_CPP_ROOT_DIR}" )
set( DOXYGEN_INPUT               "${GPUDB_API_CPP_ROOT_DIR}" )
set( DOXYGEN_FILE_PATTERNS       "*.h *.hpp" )
set( DOXYGEN_PREDEFINED          "" )

MESSAGE(STATUS "*")
ADD_DOXYGEN( doxygen
             ${GPUDB_API_CPP_ROOT_DIR}/build/CMake-doxygen.in
             ${CMAKE_BINARY_DIR}/doxygen_GPUdb_API_CPP.cfg
             ${CMAKE_BINARY_DIR} )

MESSAGE(STATUS "*")

# ===========================================================================
# Configure build and 3rd party libraries
# ===========================================================================

if (NOT DEFINED GPUDB_API_CPP_LIBS_DIR)
    set(GPUDB_API_CPP_LIBS_DIR "${GPUDB_API_CPP_SOURCE_DIR}/thirdparty/install")
endif()
set(GPUDB_API_CPP_LIBS_DIR ${GPUDB_API_CPP_LIBS_DIR} CACHE PATH
    "Specify the location of the 3rd party development libs." FORCE)

MESSAGE(STATUS "* GPUDB_API_CPP_LIBS_DIR = '${GPUDB_API_CPP_LIBS_DIR}'")

if (NOT EXISTS ${GPUDB_API_CPP_LIBS_DIR}/include/)
    MESSAGE(FATAL_ERROR "Please specify a valid -DGPUDB_API_CPP_LIBS_DIR=/path on cmd line, see above for info.")
endif()

# ---------------------------------------------------------------------------
# Find Boost lib

MESSAGE(STATUS "*")

# Need at least 1.48 version for boost::asio::connect(socket, iterator) function.
find_package(Boost 1.48 COMPONENTS system thread )

MESSAGE(STATUS "* Boost_FOUND        = ${Boost_FOUND}")
MESSAGE(STATUS "* Boost_VERSION      = ${Boost_VERSION}")
MESSAGE(STATUS "* Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
MESSAGE(STATUS "* Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
MESSAGE(STATUS "* Boost_LIBRARIES    = ${Boost_LIBRARIES}")
MESSAGE(STATUS "*")

if (NOT Boost_FOUND)
    MESSAGE(STATUS "ERROR : Unable to find Boost 'system' and 'thread' libraries. ")
    MESSAGE(STATUS "Please set one or all of the CMake variables BOOST_ROOT, BOOST_INCLUDEDIR, BOOST_LIBRARYDIR to help find it.")
    MESSAGE(STATUS "E.G. cmake -DBOOST_ROOT=/path/to/boost/install -DBOOST_INCLUDEDIR=/path/to/boost/install/include -DBOOST_LIBRARYDIR=/path/to/boost/install/lib")
    MESSAGE(FATAL_ERROR "Unable to find Boost libraries. Aborting.")
endif()

# ---------------------------------------------------------------------------
# Include and Link directories and libs.

set(GPUDB_API_CPP_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${GPUDB_API_CPP_LIBS_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

foreach( d ${GPUDB_API_CPP_INCLUDE_DIRS})
    if (NOT EXISTS ${d})
        MESSAGE(WARNING, "Missing include dir '${d}' in GPUDB_API_CPP_INCLUDE_DIRS")
    endif()
endforeach()


# Link order is from bottom up.
# i.e. Put libA that depends on libB above libB in the list below.
set(GPUDB_API_CPP_LINK_LIBRARIES
    snappy
    avrocpp # dep on boost
    ${Boost_LIBRARIES}
    #boost_system boost_thread # boost_timer boost_filesystem boost_regex boost_chrono
)

# ---------------------------------------------------------------------------
# All projects search these #include and linker paths

include_directories(SYSTEM ${GPUDB_API_CPP_INCLUDE_DIRS})

link_directories(/usr/lib/
                 /usr/lib64
                 ${GPUDB_API_CPP_LIBS_DIR}/lib
                 ${Boost_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -rdynamic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS") # For MAX_INTXX when including <stdint.h>
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -D__STDC_LIMIT_MACROS")   # For MAX_INTXX when including <stdint.h>

# ---------------------------------------------------------------------------

if (CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    string(REPLACE "\n" "" GCC_VERSION "${GCC_VERSION}")
    message(STATUS "* Compiling with GCC version ${GCC_VERSION}")

    # We ALWAYS want -Wall and -Wextra
    if (NOT "${CMAKE_CXX_FLAGS}" MATCHES "-Wall")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    endif()
    if (NOT "${CMAKE_CXX_FLAGS}" MATCHES "-Wextra")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    endif()

    # printf() is very dangerous, check usage
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2")

    # Add some more warnings, but only if the version understands them and if not already enabled.
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -Q -Wall -Wextra --help=warnings
                    OUTPUT_VARIABLE GCC_CXX_ENABLED_WARNINGS_STR)

    # -Wconversion : maybe, too many warnings
    # -Wshadow     : no, catch(const std::exception& e) e shadows something, can't tell what. too many warnings.
    # -Wsign-promo : no, too many warnings from enums upgraded to int when passed to logger's ostringstream

    # -Wdelete-non-virtual-dtor -Wnon-virtual-dtor : no, or at least not with nvcc, it doesn't understand them.
    # -Woverloaded-virtual : no, or at least not with nvcc, it doesn't understand them.
    # -Wreorder : no, or at least not with nvcc, it doesn't understand them.

    # Add to this list of -Wblah params and they'll be added if current [disabled].
    # This also handles new/old versions that don't understand that flag at all.
    set(GPUDB_API_CPP_EXTRA_CMAKE_CXX_WARNING_FLAGS
        -Wsign-compare -Wswitch -Wnarrowing -Wmissing-braces -Wparentheses -Wreturn-type
        -Wunused -Wunused-variable -Wuninitialized -Wmaybe-uninitialized -Wlogical-op)

    foreach(warn_param ${GPUDB_API_CPP_EXTRA_CMAKE_CXX_WARNING_FLAGS})
        if (NOT "${GCC_CXX_ENABLED_WARNINGS_STR}" MATCHES "${warn_param}")
            MESSAGE(STATUS "* Unknown gcc warning: ${warn_param}")
        endif()

        if ("${GCC_CXX_ENABLED_WARNINGS_STR}" MATCHES "${warn_param}[ \t]*\\[disabled\\]")
            MESSAGE(STATUS "* Adding gcc warning: ${warn_param}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warn_param}")
        endif()
    endforeach()

    # Avro uses std::auto_ptr(), silence warnings about since there's nothing we can do about it.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

endif()

# Throw out all unused linker dependencies to shared libs.
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed")

# ===========================================================================
# Print out what the compile settings are.
# ===========================================================================

get_directory_property(COMPILE_DEFS COMPILE_DEFINITIONS)
get_directory_property(INCLUDE_DIRS INCLUDE_DIRECTORIES)
get_directory_property(LINK_DIRS    LINK_DIRECTORIES)

message(STATUS "* COMPILE_DEFS    : ${COMPILE_DEFS}")
message(STATUS "* INCLUDE_DIRS    : ${INCLUDE_DIRS}")
message(STATUS "* LINK_DIRS       : ${LINK_DIRS}")
message(STATUS "* CMAKE_CXX_FLAGS : ${CMAKE_CXX_FLAGS}")

# ===========================================================================

include( build/CMakeQtCreator.cmake )
set(QT_CREATOR_INCLUDES ${INCLUDE_DIRS}
                        /usr/include
                        /usr/local/include )
ADD_QTCREATOR_PROJECT( "gpudb-api-cpp" )

# ===========================================================================
# Define the targets, the subdir CMakeLists.txt files call the functions.
# ===========================================================================

# ---------------------------------------------------------------------------
# gpudb-api-cpp library
# ---------------------------------------------------------------------------

function( gpudb_api_cpp_lib_TARGETS )

ADD_LIBRARY_FULL( gpudb_api_cpp_lib
    HEADERS
        Avro.hpp
        DynamicTableRecord.hpp
        GenericRecord.hpp
        GPUdbException.hpp
        GPUdb.hpp
        GPUdbFunctions.hpp
        GPUdbTemplates.hpp
        Type.hpp
        GPUdbIngestor.hpp
        protocol/EndpointKeywords.h
        protocol/GPUdbProtocol.h
    SOURCES
        Avro.cpp
        DynamicTableRecord.cpp
        GenericRecord.cpp
        GPUdb.cpp
        GPUdbIngestor.cpp
        GPUdbException.cpp
        Type.cpp
        protocol/EndpointKeywords.cpp
    LINK_LIBRARIES
        ${GPUDB_API_CPP_LINK_LIBRARIES}
    PROPERTIES FOLDER "Library"
    PROPERTIES COMPILE_FLAGS "-std=c++0x"
    PROPERTIES OUTPUT_NAME "gpudb_api_cpp"
)

endfunction( gpudb_api_cpp_lib_TARGETS )

# ---------------------------------------------------------------------------
# sample executable
# ---------------------------------------------------------------------------

function( gpudb_api_cpp_example_TARGETS )

ADD_EXECUTABLE_FULL( gpudb-api-example
    HEADERS
    SOURCES
        gpudb-api-example.cpp
    LINK_LIBRARIES
        gpudb_api_cpp_lib
        ${GPUDB_API_CPP_LINK_LIBRARIES}
    PROPERTIES COMPILE_FLAGS "-std=c++0x"
    PROPERTIES FOLDER "Example"
    PROPERTIES OUTPUT_NAME "gpudb-api-example"
)

endfunction( gpudb_api_cpp_example_TARGETS )


# ---------------------------------------------------------------------------
# test executable
# ---------------------------------------------------------------------------

function( gpudb_api_cpp_multihead_ingest_TARGETS )

ADD_EXECUTABLE_FULL( test_multihead_ingest
    HEADERS
    SOURCES
        test_multihead_ingest.cpp
    LINK_LIBRARIES
        gpudb_api_cpp_lib
        ${GPUDB_API_CPP_LINK_LIBRARIES}
    PROPERTIES COMPILE_FLAGS "-std=c++0x"
    PROPERTIES FOLDER "Example"
    PROPERTIES OUTPUT_NAME "test_multihead_ingest"
)

endfunction( gpudb_api_cpp_multihead_ingest_TARGETS )

# ===========================================================================
# Add the subdirectories with their projects in them
# ===========================================================================

add_subdirectory(gpudb)
add_subdirectory(example)

MESSAGE(STATUS "*")
PRINT_ADD_LIBRARY(TRUE)
PRINT_ADD_EXECUTABLE(TRUE)
