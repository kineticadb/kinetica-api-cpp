#ifndef __GPUDB__GPUDBEXCEPTION_HPP__
#define __GPUDB__GPUDBEXCEPTION_HPP__

#include "gpudb/GenericRecord.hpp"
#include "gpudb/Http.hpp"

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
    GPUdbException(const std::string& messageParam) : message( messageParam ) {}

    virtual const char* what() const throw() { return message.c_str(); }

    ~GPUdbException() throw() {}

protected:
    std::string message;
};



class GPUdbInsertionException : public GPUdbException
{
public:
    GPUdbInsertionException( const std::string& messageParam) : GPUdbException( messageParam ) {}

    GPUdbInsertionException( const std::string& url_,
                             const std::vector<gpudb::GenericRecord>& records_,
                             const std::string& message_ );

    virtual const char* what() const throw() { return message.c_str(); }

    ~GPUdbInsertionException() throw() {}

    // Insert additional records into this exception object's record queue
    void append_records( std::vector<gpudb::GenericRecord>::iterator begin,
                         std::vector<gpudb::GenericRecord>::iterator end );

    std::vector<gpudb::GenericRecord> getRecords() const { return records; }

private:
    std::string url;
    std::vector<gpudb::GenericRecord> records;
};


class GPUdbExitException : public GPUdbException
{
public:
    GPUdbExitException( const std::string& messageParam) : GPUdbException( messageParam ) {}

    virtual const char* what() const throw() { return message.c_str(); }

    ~GPUdbExitException() throw() {}
};


class GPUdbHAUnavailableException : public GPUdbException
{
public:
    GPUdbHAUnavailableException( const std::string& messageParam) : GPUdbException( messageParam ) {}

    GPUdbHAUnavailableException( const std::string& message_,
                                 const std::vector<HttpUrl>& urls_ );

    virtual const char* what() const throw() { return message.c_str(); }

    ~GPUdbHAUnavailableException() throw() {}


private:
    std::vector<HttpUrl> urls;
};


class GPUdbSubmitException : public GPUdbException
{
public:
    GPUdbSubmitException( const std::string& messageParam) : GPUdbException( messageParam ) {}

    GPUdbSubmitException( HttpUrl url_, const std::vector<uint8_t>& request_,
                          const std::string& message_);

    virtual const char* what() const throw() { return message.c_str(); }

    ~GPUdbSubmitException() throw() {}


private:
    std::vector<uint8_t>  request;
    HttpUrl               url;
};


}

#endif
