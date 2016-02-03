# The makefile

# Override this variable on the command line to point to the root directory
# of gpudb-core-libs install dir that has the header and libs for
# avrocpp, boost, and snappy.
GPUDB_CORE_LIBS_DIR := /opt/gpudb-core-libs


all: gpudb-api-example.cpp
	g++ -o gpudb-api-example gpudb-api-example.cpp ../src/Avro.cpp ../src/GPUdb.cpp ../src/GPUdbException.cpp ../src/Type.cpp ../src/DynamicTableRecord.cpp ../src/GenericRecord.cpp -I${GPUDB_CORE_LIBS_DIR}/include/ -L${GPUDB_CORE_LIBS_DIR}/lib/ -lavrocpp -lboost_system -lboost_thread -lsnappy -Wall -m64 -Wl,-rpath=${GPUDB_CORE_LIBS_DIR}/lib/

