#ifndef __GPUDB__GPUDBEXCEPTION_HPP__
#define __GPUDB__GPUDBEXCEPTION_HPP__

#include "gpudb/GenericRecord.hpp"

#include <exception>
#include <string>
#include <vector>


namespace gpudb
{
    /*
     * Generic exceptions thrown by the C++ GPUdb client API.
     */
    class GPUdbException : public std::exception
    {
        public:
            GPUdbException(const std::string& messageParam) : message(messageParam) {}

            virtual const char* what() const throw() { return message.c_str(); }

            ~GPUdbException() throw() {}

        private:
            std::string message;
    };



    class GPUdbInsertionException : public GPUdbException
    {
        public:
            GPUdbInsertionException( const std::string& messageParam) : GPUdbException(messageParam) {}

            GPUdbInsertionException( const std::string& url_,
                                     const std::vector<gpudb::GenericRecord>& records_,
                                     const std::string& message_ );

            virtual const char* what() const throw() { return message.c_str(); }

            ~GPUdbInsertionException() throw() {}

            // Insert additional records into this exception object's record queue
            void append_records( std::vector<gpudb::GenericRecord>::iterator begin,
                                 std::vector<gpudb::GenericRecord>::iterator end );

        private:
            std::string message;
            std::string url;
            std::vector<gpudb::GenericRecord> records;
    };


}

#endif
