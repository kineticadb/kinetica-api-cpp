
project( gpudb_api_cpp_lib C CXX ) # sets ${XXX_SOURCE_DIR} and ${XXX_BINARY_DIR}

gpudb_api_cpp_lib_TARGETS(STATIC)
if (NOT WIN32)
    gpudb_api_cpp_lib_TARGETS(SHARED)
endif()

