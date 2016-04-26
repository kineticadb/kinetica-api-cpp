#ifndef __GPUDB__GPUDBEXCEPTION_HPP__
#define __GPUDB__GPUDBEXCEPTION_HPP__

#include <exception>
#include <string>

namespace gpudb
{
    /*
     * Generic exceptions thrown by the C++ GPUdb client API.
     */
    class GPUdbException : public std::exception
    {
        public:
            GPUdbException(const std::string& message);
            virtual const char* what() const throw();
            ~GPUdbException() throw();

        private:
            std::string message;
    };


}

#endif
