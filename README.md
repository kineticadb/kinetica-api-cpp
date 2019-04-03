Kinetica C++ API
================

This project contains the source code of the C++ Kinetica API.

The documentation can be found at http://www.kinetica.com/docs/6.2/index.html.  The
C++ specific documentation can be found at:

*   https://www.kinetica.com/docs/tutorials/cpp_guide.html
*   https://www.kinetica.com/docs/api/c++/annotated.html

For the client-side API changes, please refer to CHANGELOG.md, and for GPUdb
function changes, please refer to CHANGELOG-FUNCTIONS.md.


Instructions to build the Kinetica C++ API on Linux
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
        -   Debian based distributions install it using:
            -   `apt install cmake`
    -   Boost library v1.48 or newer, http://www.boost.org/
        -   The GPUdb API depends on the Boost 'regex', 'system' and 'thread' libraries.
        -   It is easiest to use the Linux distribution provided Boost package.
        -   Red Hat based distributions install it using:
            -   `yum install boost boost-devel boost-system boost-thread boost-regex`
            -   The headers are typically installed in /usr/include/boost and libs in /usr/lib64
        -   Red Hat and CENTOS 6 ship with an older boost 1.41 library.
            Use the 'EPEL' (https://fedoraproject.org/wiki/EPEL) repository to
            install the newer boost148-* libraries into the
            /usr/include/boost148/ and /usr/lib64/boost148/ directories.
            See below about how to configure the GPUdb CPP API CMake build to detect
            and use an alternate boost install.
        -   Debian based distributions install it using:
            -   `apt install libboost-dev libboost-regex-dev libbost-system-dev libboost-thread-dev`
-   Build the libraries from the source archives in the thirdparty/ directory.
    -   Dependencies
        - CMake (as above)
        - ZLib and BZ2 - compiled in Boost iostreams and required in Avro
            -   Debian:
                -   `apt install libbz2-dev zlib1g-dev`
            - Red Hat based distributions:
                -   `yum install bzip2 bzip2-devel zlib zlib-devel`
        - chrpath - used to update rpaths
            -   Red Hat:
                -   `yum install chrpath`
            -   Debian:
                -   `apt install chrpath`
    -   Before running the build script, locate the version of Boost that you want
        to use to build the package.  You can download a source tarball of Boost from
        http://www.boost.org.  You can place the tarball in the thirdparty folder or
        specify the location using the --boost-archive parameter to the build script.
    -   The `thirdparty/build-thirdparty-libs.sh` script builds the provided version
        of Boost, and the included Apache Avro and Google Snappy in the directory 
        '../_build/thirdparty/build' and installs them to '../_build/thirdparty/install'.
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
    -   Simply delete the '../_build/thirdparty/build' and '../_build/thirdparty/install' 
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
    -   `-DGPUDB_NO_HTTPS=TRUE`
        -   Disable SSL support.  Default value is `FALSE` which enables SSL support.
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


Instructions to build the Kinetica C++ API on Windows
=====================================================


Requirements
------------

-	Windows 7, 8, 10, or more recent
-	Microsoft Visual Studio 2013 or more recent
-	Third-party libraries (see below for building instructions)


Notes
-----

-	Windows version of the GPUdb C++ API does not support SSL.  You will need to define the GPUDB_NO_HTTPS macro before including API header files.
-	To automatically restore NuGet projects (needed for openssl), open Tools > Options > NuGet Package Manager and select the options for Allow NuGet to download missing packages and Automatically check for missing packages during build in Visual Studio
	-	See: https://docs.microsoft.com/en-us/nuget/Consume-Packages/Package-restore-troubleshooting


Third-party Libraries
---------------------

-	Install/Build these libraries from their distribution packages:
    -	Boost library v1.65.1: http://www.boost.org/
        -	The GPUdb API depends on the Boost 'regex', 'system' and 'thread' libraries.
		-	Download Windows ZIP from: www.boost.org/users/history/version_1_65_1.html (or whatever version of Boost is needed)
			-	Full build instructions are in this file at: more/getting_started/windows.html (see section 5)
			-	Or, you may try to use the "Prebuilt windows binaries"
			-	Or, you may be able to use the NuGet Boost Package(s)
				-	From the NuGet Console: Install-Package boost -Version 1.65.1
				-	For VS 2013 Libraries: Install-Package boost-vc120 -Version 1.65.1
				-	For VS 2015 Libraries: Install-Package boost-vc140 -Version 1.65.1
				-	You may need to update the Project paths
		-	Uncompress to: gpudb-api-cpp\thirdparty\boost_1_65_1.
			-	You should now have a file: gpudb-api-cpp\thirdparty\boost_1_65_1\bootstrap.bat
			-	The header files should now be in the right place, but you still need to build the libraries.
		-	From your Widows Start menu, find the entry under VS for "Developer Command Prompt for VS2013" (or the target VS version)
		-	Open that to create a VS build command prompt window.
		-	CD to <your machine path>\gpudb-api-cpp\thirdparty\boost_1_65_1
		-	Run: bootstrap.bat
			-	You should now have a file: gpudb-api-cpp\thirdparty\boost_1_65_1\b2.exe
		-	Run: b2 address-model=64 --build-type=complete --toolset=msvc-12.0 --threading=multi --runtime-link=shared --stagedir=./lib64
			-	Change the "toolset" parameter for different versions of VS.
				-	Use "--toolset=msvc-14.0" for VS 2015.
				-	Use "--toolset=msvc-14.1" for VS 2017.
			-	Use "address-model=32" for x86 (and change --stagedir=./lib32).
			-	Use "--variant=debug" for DEBUG builds or "--variant=release" for RELEASE builds.  You may have to change "--build-type" to "minimal" for this to take effect.
			-	The libraries should end up in: gpudb-api-cpp/thirdparty/boost_1_65_1/lib64/lib
			-	For other options, run: b2 --help
	-	Avro
		-	Download avro-cpp-1.7.7.tar.gz.  See: http://www.apache.org/dyn/closer.cgi/avro/
		-	Full instructions at: https://avro.apache.org/docs/current/api/cpp/html/index.html
		-	Uncompress to: gpudb-api-cpp\thirdparty\avro-cpp-1.7.7
            Note: The C++ API repository already contains the avro-cpp-1.7.7 directory which works
            out-of-the-box for linux; but for Windows, delete that directory and then uncompress
            the tar.gz file.
		-	Copy "api" folder to "avro" (do not rename the directory; make a copy as the "api" directory
            will be required later as well)
			-	Building Avro requires CMake.  You can install this from the NuGet Console by typing: Install-Package CMake -Version 3.5.2
			-	Add it to your path: gpudb-api-cpp\project\gpudbapidll\packages\CMake.3.5.2\bin (or, wherever you installed it)
            -   If the NuGet Console fails to install CMake, then download and run the latest CMake Windows installer
                file from https://cmake.org/download/ (cmake-3.xx.y-win32-x86.msi for 32-bit Windows and
                cmake-3.xx.y-win64-x64.msi for 64-bit Windows).
			-	In the VS Build Command-Prompt, set the environment variable BOOST_ROOT:
                > set BOOST_ROOT=<your_path>gpudb-api-cpp\thirdparty\boost_1_65_1
			-	Similarly, set BOOST_INCLUDEDIR=<your_path>gpudb-api-cpp\thirdparty\boost_1_65_1\boost
			-	Also, set BOOST_LIBRARYDIR=<your_path>gpudb-api-cpp\thirdparty\boost_1_65_1\lib64\lib
		-	In the same VS Build Command-Prompt, change to the Avro folder: gpudb-api-cpp\thirdparty\avro-cpp-1.7.7
		-	Run: cmake -G "Visual Studio 10"
			-	Change "10" to "14" for VS 2015
			-	This creates, among other things, Avro-cpp.sln file.
		-	Open the Avro-cpp.sln file in Visual Studio
			-	If needed, create an x64 platform in Configuratino Manager
			-	You should only need to build the "avrocpp_s" project
			-	In avrocpp_s Property Pages, remove x86 option from "Additional Options" on the Librarian-All Options page
			-	In avrocpp_s Property Pages, Configuration Properties-General, change Output Directory to: $(SolutionDir)$(Platform)\$(Configuration)\
		-	Build desired flavors of libraries (of avrocpp_s Project)
-	Open gpudb-api-cpp\project\gpudbapidll\gpudbapidll.sln
	-	To build the C++ API with support for SSL, you will need to install OpenSSL
		-	You can install this from the NuGet Console (for the gpudbapiexe project) by typing: Install-Package zeroc.openssl.v120 -Version 1.0.2.4
		-	You may need to install the appropriate version for your Visual Studio (e.g., v140 for VS 2015)
	-	If you do not need SSL support, you may add GPUDB_NO_HTTPS to the Proprocessor Definitions
    -   Build your desired flavors (e.g., Debug/Release, x64/x86)
