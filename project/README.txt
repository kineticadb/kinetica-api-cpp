a) You should have fetched the entire bitbucket gpudb-api-cpp project. Should find example, project, src and thirdaprty directories underneath.

b) thirdparty has a README.txt which states that you will have to download boost and copy it under the thirdparty dir. Tested with boost_1_59_0.

c) Under <sandbox>\gpudb-api-cpp\project\gpudbapidll, you will see gpudbapidll.sln. This solution has been tested on 2013 VS community edition.

d) A bunch of include dirs and library search dirs contain hardcoded path. Will make ENV vars when I have time. But you may have to change the C++ and Linker
property sheet to change the dirs.

e) Do a full build on the project gpudbapilib. This should create <sndbox>\gpudb-api-cpp\project\gpudbapidll\x64\Release\gpudbapilib.lib.

f) Do a full build on the project gpudbapiexe. This should create <sndbox>\gpudb-api-cpp\project\gpudbapidll\x64\Release\gpudbapiexe.exe. This test code is
in gpudbapiexe.cpp. Make sure that the URL etc is correct and also that the set for which the schema is being referenced in the code is available on the 
GPUdb you are accessing. For example, right now it wants a set called TaxiTrack under MASTER on GPUdb.