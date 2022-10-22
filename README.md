<h3 align="center" style="margin:0px">
	<img width="200" src="https://2wz2rk1b7g6s3mm3mk3dj0lh-wpengine.netdna-ssl.com/wp-content/uploads/2018/08/kinetica_logo.svg" alt="Kinetica Logo"/>
</h3>
<h5 align="center" style="margin:0px">
	<a href="https://www.kinetica.com/">Website</a>
	|
	<a href="https://docs.kinetica.com/7.1/">Docs</a>
	|
	<a href="https://docs.kinetica.com/7.1/api/c++/">API Docs</a>
	|
	<a href="https://join.slack.com/t/kinetica-community/shared_invite/zt-1bt9x3mvr-uMKrXlSDXfy3oU~sKi84qg">Community Slack</a>   
</h5>


# Kinetica C++ API

-  [Overview](#overview)
-  [Building the API (Linux)](#building-the-api-linux)
-  [Building the API (Windows)](#building-the-api-windows)
-  [Support](#support)
-  [Contact Us](#contact-us)
 

## Overview

This project contains the source code for the C++ Kinetica API.

The C++ API is supported on both Linux-based systems and Windows.

* [Building the API (Linux)](#building-the-api-linux)
* [Building the API (Windows)](#building-the-api-windows)

The following links will provide the information needed to get started with
Kinetica and the C++ API:

* [C++ API documentation](https://docs.kinetica.com/7.1/api/c++/)
* [C++ API tutorial](https://docs.kinetica.com/7.1/guides/cpp_guide/)
* [Kinetica documentation site](https://docs.kinetica.com/7.1/)

For changes to the client-side API, please refer to
[CHANGELOG.md](CHANGELOG.md).  For
changes to Kinetica functions, please refer to
[CHANGELOG-FUNCTIONS.md](CHANGELOG-FUNCTIONS.md).


## Building the API (Linux)

### Install Dependencies

RHEL:

    yum update -y
    yum install -y epel-release
    yum install -y which gcc gcc-c++ cmake make bzip2 bzip2-devel zlib zlib-devel chrpath

Ubuntu:

    apt-get update -y
    apt-get install -y which gcc gcc-c++ cmake make libbz2-dev zlib1g-dev chrpath

### Clone the Repository

In the desired directory, run:

    git clone https://github.com/kineticadb/kinetica-api-cpp.git && cd kinetica-api-cpp/

### Building Thirdparty Libraries

The C++ API requires several local thirdparty library archives: Boost (v1.56.0
or later), Avro (v1.7.7), Snappy Compression (v1.1.3). Both Avro and Snappy
Compression archives are included by default in the ``thirdparty`` directory of
the repository. A Boost archive must be downloaded to the ``thirdparty``
directory before ``./build-thidparty-libs.sh`` can be run. For example, to
download Boost v1.71.0:

    wget -P thirdparty/ https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2

Build the thirdparty libraries:

    thirdparty/build-thirdparty-libs.sh

### Building the API

There are four CMake build types: ``Debug``, ``RelWithDebInfo``, ``Release``,
and ``MinSizeRel``. Kinetica recommends using ``RelWithDebInfo`` (the default)
as it provides good performance and debug information.

There are several flags available to CMake for the C++ API:

**NOTE:** These flags are passed in as CMake cache entries, so they must be
prepended with ``-D``.

| Flag                         | Description
| :---                         | :---
| ``GPUDB_NO_HTTPS``           | Boolean value to enable SSL support for the API if SSL is enabled for the database. If ``TRUE``, disables HTTPS support; default is ``FALSE``
| ``GPUDB_API_CPP_LIBS_DIR``   | Absolute filepath to the C++ API thirdparty library install directory (``../kinetica-api-cpp/_build/thirdparty/install/``)
| ``BOOST_ROOT``               | Absolute filepath to the built and installed Boost library directory
| ``BOOST_INCLUDEDIR``         | Absolute filepath to the Boost library's ``include`` directory (containing all of Boost's dependencies)
| ``BOOST_LIBRARYDIR``         | Absolute filepath to the directory containing all the Boost libraries

Create a build directory in the root of the API repository (preferably with a
descriptive name) and change into it:

    mkdir build-RelWithDebInfo && cd build-RelWithDebInfo

Inside the build directory, run ``cmake`` with the necessary flags and a path
to the root of the API repository.

**WARNING:** Do not run ``cmake`` in the root of the API repository.

For example:

    cmake -DGPUDB_NO_HTTPS=TRUE \
    -DGPUDB_API_CPP_LIBS_DIR=/home/kinetica-api-cpp/_build/thirdparty/install/ \
    -DBOOST_ROOT=/home/kinetica-api-cpp/_build/thirdparty/boost_1_71_0 \
    -DBOOST_INCLUDEDIR=/home/kinetica-api-cpp/_build/thirdparty/boost_1_71_0/boost/ \
    -DBOOST_LIBRARYDIR=/home/kinetica-api-cpp/_build/thirdparty/install/lib/ \
    ../

### Compiling Example Executables

In the desired build directory, run the following where *<n>* is the number of
processors on your machine:

    make -j<n>

This will compile two executables by default: an C++ API tutorial example and a
multihead ingest example. The executables can be found within the ``bin``
directory of the build folder.

### Using QTCreator

The CMake build automatically generates the necessary QTCreator project files.
If you desire to use QTCreator as an editor, then run:

    qtcreator <build-directory>/qtcreator-gpudb-api-cpp.creator

**NOTE:** To correct OpenGL crash errors when starting QTCreator, instead run:

    qtcreator -noload Welcome <build-directory>/qtcreator-gpudb-api-cpp.creator

## Building the API (Windows)

### Prerequisites

* Windows 7, 8, 10, or later
* Microsoft Visual Studio 2013 or later
* CMake 3.5.2 or later

### Clone the Repository

In the desired directory, run:

    git clone https://github.com/kineticadb/kinetica-api-cpp.git && cd kinetica-api-cpp\

### Building Thirdparty Libraries

The C++ API requires you to build and install several thirdparty libraries:
Boost (v1.65.1 or later), Avro (v1.7.7), Snappy Compression (v1.1.3). Both Avro
and Snappy Compression archives are included and unzipped by default in the
``thirdparty`` directory of the repository, but the unzipped Avro archive may
not work for Windows.

#### Boost

1. Download a Windows .zip Boost archive from the [official Boost repository](https://www.boost.org/users/download/)
2. Uncompress the Boost archive to the ``thirdparty`` directory
3. From the Visual Studio build command prompt window:
   1. Change directory into the ``thirdparty/boost_<version>`` directory, e.g., ``boost_1_65_1``
   2. Run ``bootstrap.bat``
   3. Run the appropriate ``b2`` command based on your Windows version and
      desired build, replacing *<n>* with ``0`` if using Virtual Studio 2015 or
      with ``1`` if using Virtual Studio 2017:

          \# 64-bit DEBUG
          b2 address-model=64 --build-type=minimal --toolset=msvc-12.0 --threading=multi --runtime-link=shared --stagedir=./lib64 --toolset=msvc-14.<n> --variant=debug

          \# 64-bit RELEASE
          b2 address-model=64 --build-type=minimal --toolset=msvc-12.0 --threading=multi --runtime-link=shared --stagedir=./lib64 --toolset=msvc-14.<n> --variant=release

          \# 32-bit DEBUG
          b2 address-model=32 --build-type=minimal --toolset=msvc-12.0 --threading=multi --runtime-link=shared --stagedir=./lib32 --toolset=msvc-14.<n> --variant=debug

          \# 32-bit RELEASE
          b2 address-model=32 --build-type=minimal --toolset=msvc-12.0 --threading=multi --runtime-link=shared --stagedir=./lib32 --toolset=msvc-14.<n> --variant=release

The libraries will be available in the following directory: ``kinetica-api-cpp/thirdparty/boost_<version>/lib64/lib``

**TIP:** If building Boost is not desired, there are unofficial prebuilt Windows
Boost binaries available or Boost is able to be installed via NuGet. To install
via the NuGet Console:

    Install-Package boost -Version 1.65.1

Note to change ``boost`` to ``boost-vc120`` to include Virtual
Studio 2013 libraries or ``boost-vc140`` to include Virtual Studio 2015
libraries. You may need to update the project paths.

#### Avro

1. Remove the ``kinetica-api-cpp/thirdparty/avro-cpp-1.7.7`` directory
2. In the ``thirdparty`` directory, uncompress the included Avro archive
3. Copy the ``api`` directory to the ``avro-cpp-1.7.7/avro`` directory
4. From the Visual Studio build command prompt window:
   1. Set the following environment variables:

          BOOST_ROOT=kinetica-api-cpp\thirdparty\boost_<version>
          BOOST_INCLUDEDIR=kinetica-api-cpp\thirdparty\boost_<version>\boost
          BOOST_LIBRARYDIR=kinetica-api-cpp\thirdparty\boost_<version>\lib64\lib

   2. Change into the ``avro-cpp-1.7.7`` directory
   3. Run, replacing *<version>* with the version of Visual Studio currently 
      being used:

          cmake -G "Visual Studio <version>"

5. After a successful build, open the new ``avro-cpp.sln`` file in Visual Studio
6. Optionally, if using 64-bit windows, create an x64 platform in Configuration
   Manager
7. In the ``avrocpp_s`` project in Visual Studio:
   1. Under *Properties* > *Librarian - All Options* > *Additional Options*,
      remove the ``x86`` option
   2. Under *Properties* > *Configuration Properties - General*, change the
      output directory:

          $(SolutionDir)$(Platform)\$(Configuration)\

   3. Build the desired libraries

### Building the API

1. In Visual Studio, open the
   ``kinetica-api-cpp\project\gpudbapidll\gpudbapidll.sln`` project
2. Optionally, if building the C++ API with SSL support, install OpenSSL from
   the NuGet Console for the ``gpudbapiexe`` project, replacing *<version>* with
   the appropriate Visual Studio version:

      Install-Package zeroc.openssl.v<version> -Version 1.0.2.4

3. If enabling SSL support, navigate to *Tools* > *Options* >
   *NuGet Package Manager* and select the options for allowing NuGet to download
   missing packages and automatically check for missing packages during builds.
   Review [Package Restore troubleshooting](https://docs.microsoft.com/en-us/nuget/Consume-Packages/Package-restore-troubleshooting)
   for more information
4. Build the desired API release (debug, release, x64, x86, etc.)

**TIP:** If you do not need SSL support, add ``GPUDB_NO_HTTPS`` to the
preprocessor definitions


## Support

For bugs, please submit an
[issue on Github](https://github.com/kineticadb/kinetica-api-cpp/issues).

For support, you can post on
[stackoverflow](https://stackoverflow.com/questions/tagged/kinetica) under the
``kinetica`` tag or
[Slack](https://join.slack.com/t/kinetica-community/shared_invite/zt-1bt9x3mvr-uMKrXlSDXfy3oU~sKi84qg).


## Contact Us

* Ask a question on Slack:
  [Slack](https://join.slack.com/t/kinetica-community/shared_invite/zt-1bt9x3mvr-uMKrXlSDXfy3oU~sKi84qg)
* Follow on GitHub:
  [Follow @kineticadb](https://github.com/kineticadb) 
* Email us:  <support@kinetica.com>
* Visit:  <https://www.kinetica.com/contact/>
