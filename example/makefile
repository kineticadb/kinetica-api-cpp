# The makefile

# Override this variable on the command line to point to the root directory
# of gpudb-core-libs install dir that has the header and libs for
# avrocpp, boost, and snappy.
GPUDB_CORE_LIBS_DIR := /opt/gpudb-core-libs


all: gpudb-api-example gpudb-join-example

gpudb-api-example: makefile gpudb-api-example.cpp
	g++ -o gpudb-api-example gpudb-api-example.cpp ../gpudb/Avro.cpp ../gpudb/ColumnProperties.cpp ../gpudb/GPUdb.cpp ../gpudb/GPUdbException.cpp ../gpudb/Http.cpp ../gpudb/Type.cpp ../gpudb/GenericRecord.cpp -I${GPUDB_CORE_LIBS_DIR}/include/ -I.. -L${GPUDB_CORE_LIBS_DIR}/lib/ -lavrocpp -lboost_system -lboost_thread -lpthread -lsnappy -lcrypto -lssl -Wall -m64 -Wl,-rpath=${GPUDB_CORE_LIBS_DIR}/lib/

gpudb-join-example: makefile gpudb-join-example.cpp
	g++ -o gpudb-join-example gpudb-join-example.cpp ../gpudb/Avro.cpp ../gpudb/ColumnProperties.cpp ../gpudb/GPUdb.cpp ../gpudb/GPUdbException.cpp ../gpudb/Http.cpp ../gpudb/Type.cpp ../gpudb/GenericRecord.cpp -I${GPUDB_CORE_LIBS_DIR}/include/ -I.. -L${GPUDB_CORE_LIBS_DIR}/lib/ -lavrocpp -lboost_system -lboost_thread -lpthread -lsnappy -lcrypto -lssl -Wall -m64 -Wl,-rpath=${GPUDB_CORE_LIBS_DIR}/lib/
