
SOURCES := dev_guide.cpp

OBJS := $(SOURCES:.cpp=.o)

INCLUDE_PATH=-I../../src -I/opt/gpudb-core-libs/include

LIBS=-L../../build-RelWithDebInfo/lib/RelWithDebInfo -lgpudb_api_cpp -L/opt/gpudb-core-libs/lib -lavrocpp -lboost_system -lboost_thread

CFLAGS=-Wall

LFLAGS=-Wl,-rpath=/opt/gpudb-core-libs/lib:../../build-RelWithDebInfo/lib/RelWithDebInfo


all: dev_guide

clean:
	rm -fr $(OBJS) dev_guide


dev_guide:  $(OBJS)
	$(CXX)  $(CFLAGS) -o $@  $(OBJS) $(LFLAGS) $(LIBS)


.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) $(INCLUDE_PATH)  -c $<


