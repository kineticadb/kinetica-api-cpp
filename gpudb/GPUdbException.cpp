#include "gpudb/GPUdbException.hpp"

namespace gpudb
{
    GPUdbException::GPUdbException(const std::string& message):
        message(message)
    {
    }

    const char* GPUdbException::what() const throw()
    {
        return message.c_str();
    }

    GPUdbException::~GPUdbException() throw()
    {
    }
}
