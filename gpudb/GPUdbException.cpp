#include "gpudb/GPUdbException.hpp"

#include <sstream>



namespace gpudb
{

GPUdbInsertionException::GPUdbInsertionException( const std::string& url_,
                                                  const std::vector<gpudb::GenericRecord>& records_,
                                                  const std::string& message_ ) :
    GPUdbException( message_ )
{
    url = url_;
    message = message_;

    // Save the records
    this->records.reserve( records_.size() );
    this->records.assign( records_.begin(), records_.end() );
}

/*
 * Insert additional records into this exception object's record queue
 */
void GPUdbInsertionException::append_records( std::vector<gpudb::GenericRecord>::iterator begin,
                                              std::vector<gpudb::GenericRecord>::iterator end )
{
    std::vector<gpudb::GenericRecord>::iterator it;
    for ( it = begin; it != end; ++it )
    {
        this->records.push_back( *it );
    }
}  // end append_records



GPUdbHAUnavailableException::GPUdbHAUnavailableException( const std::string& message_,
                                                          const std::vector<HttpUrl>& urls_ ) :
    GPUdbException( message_ )
{
    urls = urls_;

    std::ostringstream oss;
    oss << message_ << " URLs:";
    for ( size_t i = 0; i < urls.size(); ++i )
    {
        oss << " " << urls[ i ];
    }
    message = oss.str();
}


GPUdbSubmitException::GPUdbSubmitException( HttpUrl url_, const std::vector<uint8_t>& request_,
                                            const std::string& message_ ) :
    GPUdbException( message_ )
{
    message = message_ + "; URL: " + url_.getUrl();
    url     = url_;
    request = request_;
}


}  // namespace gpudb
