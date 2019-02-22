#ifndef __GPUDB_UTILS_H__
#define __GPUDB_UTILS_H__


#include <sstream>


namespace gpudb
{



// Useful String Macros
// ----------------------
// Create an inline std::stringstream stream object and return the
// std::string from it.
// E.g. std::string msg( KINETICA_STREAM_TO_STRING( "Hello " << user_name << ", how are you?" ) );
// Note: Extra cast needed for gcc 4.8.7 (at least).
#define GPUDB_STREAM_TO_STRING(...) ( static_cast<const std::ostringstream&> (std::ostringstream() << __VA_ARGS__).str() )

// Create an inline std::stringstream stream object and return the
// char array from it.
// E.g. std::string msg( KINETICA_STREAM_TO_STRING( "Hello " << user_name << ", how are you?" ) );
#define GPUDB_STREAM_TO_CSTRING(...) ( GPUDB_STREAM_TO_STRING(__VA_ARGS__).c_str() )



} // end namespace gpudb


#endif // __GPUDB_UTILS_H__
