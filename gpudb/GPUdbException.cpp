#include "gpudb/GPUdbException.hpp"

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

}  // namespace gpudb
