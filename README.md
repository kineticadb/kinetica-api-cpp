Kinetica C++ API
================

This project contains the source code of the C++ Kinetica API.

The documentation can be found at http://www.kinetica.com/docs/5.2/index.html.  The
C++ specific documentation can be found at:

*   http://www.kinetica.com/docs/5.2/tutorials/cpp_guide.html
*   http://www.kinetica.com/docs/5.2/api/c++/annotated.html

For the client-side API changes, please refer to CHANGELOG.md, and for GPUdb
function changes, please refer to CHANGELOG-FUNCTIONS.md.

Instructions to build the Kinetica CPP API on Linux
===================================================

Please completely read this set of instructions thoroughly before starting.


Requirements
------------

-   Linux OS; RHEL, CENTOS, SLES, Ubuntu...
-   Third-party libraries (see below for building instructions)


Third-party Tools and Libraries
-------------------------------

-   Install these programs and libraries from the Linux distribution packages.
    -   CMake v2.8.4 or newer, http://www.cmake.org
        -   The Avro library and the GPUdb API have build scripts that require
            the CMake build system to generate the Makefile.
        -   It is easiest to use the Linux distribution provided cmake package.
        -   Red Hat based distributions install it using:
            -   `yum install cmake`
    -   Boost library v1.48 or newer, http://www.boost.org/
        -   The GPUdb API depends on the Boost 'system' and 'thread' libraries.
        -   It is easiest to use the Linux distribution provided Boost package.
        -   Red Hat based distributions install it using:
            -   `yum install boost boost-devel boost-system boost-thread`
            -   The headers are typically installed in /usr/include/boost and libs in /usr/lib64
        -   Red Hat and CENTOS 6 ship with an older boost 1.41 library.
            Use the 'EPEL' (https://fedoraproject.org/wiki/EPEL) repository to
            install the newer boost148-* libraries into the
            /usr/include/boost148/ and /usr/lib64/boost148/ directories.
            See below about how to configure the GPUdb CPP API CMake build to detect
            and use an alternate boost install.
-   Build the libraries from the source archives in the thirdparty/ directory.
    -   The `thirdparty/build-thirdparty-libs.sh` script builds
        Apache Avro and Google Snappy in the directory 'thirdparty/build' and
        installs them to 'thirdparty/install'.
    -   Run the script without arguments, unless you need to specify the boost root
        as shown below.
    -   Apache Avro serialization library v1.7.7, https://avro.apache.org/index.html
        -   Note that Avro depends on a valid Boost development library, see above.
        -   If you need to specify an alternate location of the boost headers
            and libraries you will need to set the environment variable 
            GPUDB_AVRO_CMAKE_FLAGS before running `build-thirdparty-libs.sh`.
            -   Oftentimes the cmake BOOST_ROOT flag is enough, but if not,
                you will need to completely specify the location of boost.
            -   Ex. GPUDB_AVRO_CMAKE_FLAGS="-DBOOST_ROOT=/path/to/boost/install" ./build-thirdparty-libs.sh
            -   Ex. GPUDB_AVRO_CMAKE_FLAGS="-DBoost_NO_BOOST_CMAKE=TRUE -DBOOST_INCLUDEDIR=/usr/include/boost148 -DBOOST_LIBRARYDIR=/usr/lib64/boost148/" ./build-thirdparty-libs.sh
    -   Snappy compression library v1.1.3, https://github.com/google/snappy
        -   The snappy library should not need any configuration.
    -   Simply delete the 'thirdparty/build' and 'thirdparty/install' 
        directories if you need to recreate them.


CMake Build Instructions
------------------------

-   There are four CMake build types: 'Debug, RelWithDebInfo, Release, MinSizeRel'
    -   The default RelWithDebInfo build offers good performance while providing
        debug information.
    -   Note that 'RelWithDebInfo' can be converted to 'Release' by
        running the `strip` program on the executable.
-   Create an "out-of-tree" directory for the build in the top-level
    directory of the 'gpudb-api-cpp' project (where this file is).
    -   Do not run CMake in the gpudb-api-cpp/ directory as it is not allowed
        and you will be required to delete any created files specified in the
        error message before being able to build anywhere else.
    -   Choose a descriptive directory name to allow for multiple build directories
        and to allow you to immediately know their configuration.
    -   This separate build directory can be deleted whenever it is no longer needed.
    -   You cannot rename this directory after running cmake, create a new one.
    -   Ex. `$ mkdir build-RelWithDebInfo`
-   The most common CMake settings to change will be these: (defaults shown below)
    -   `-DGPUDB_API_CPP_LIBS_DIR=/path/to/gpudb-api-cpp/thirdparty/install`
        -   Specify the root of the built and installed thirdparty libs.
        -   If you followed the directions to build the thirdparty libaries above
            you should not have to specify this directory.
-   Finding the Boost headers and libraries.
    -   Red Hat 7 and newer distributions should come with a suitable system
        boost development library that will be detected and used without intervention.
    -   Alternative installs of Boost will require one or more of the flags below.
    -   `-DBOOST_ROOT=/path/to/boost/install`
        -   Specify the root of the built and installed boost library.
    -   On RedHat and CENTOS 6 using the 'EPEL' boost148-* rpms use these flags.
        -   `-DBoost_NO_BOOST_CMAKE=TRUE -DBOOST_INCLUDEDIR=/usr/include/boost148 -DBOOST_LIBRARYDIR=/usr/lib64/boost148/`
        -   Which doesn't attempt to autofind boost and explicitly specifies the include and lib directories.
-   Change into the newly created and appropriately named
    `build-RelWithDebInfo` directory.
-   Run cmake in the build directory, specifying any extra options and the 
    root of the source directory.
    -   Ex. `$ cmake ../`
    -   Run the same cmake command a second time to ensure things are setup correctly.
    -   Take careful note of the information displayed as there are other options that
        may be of interest or even required for your system.
    -   If you need to rerun CMake again for any reason you do not need to
        specify the options as they are cached, but you do need to
        specify the source directory, i.e. `$ cmake ../`.
    -   You can also change one or more of the options and rerun cmake to change
        the build settings, within reason.
        -   Create a new build dir if build options are drasticly different.
-   Note that the embedded executable build time and version is ONLY updated when cmake is run.
-   To build the executables; run `$ make -j8` where `8` would be the number of processors on your machine.
-   The executables are created in the build subdirectory 'bin/RelWithDebInfo'.
-   There are a variety of make targets, run `$ make help` to view them.


Editing and Building GPUdb API C++ with QTCreator
-------------------------------------------------

-   The CMake build automatically generates the necessary QTCreator project files.
-   Run `$ qtcreator build-RelWithDebInfo/qtcreator-gpudb-api-cpp.creator`
-   Modify the build to use more processors, i.e. `make -j8` where 8 is the
    number of processors to use to compile, see the file '/proc/cpuinfo'.
    -   Open the 'Projects' near the top of the left-hand sidebar.
    -   Open the 'Details' of the 'Make' 'Build Step' and add '-j8' to the
        'Make arguments' text box.
-   The hammer in the lower left is the build button...
-   Fixing a 'Could not initialize GLX', an OpenGL crash error when starting QTCreator
    -   `$ qtcreator -noload Welcome qtcreator-gpudb-api-cpp.creator`

