#ifndef __GPUDB_INGESTOR_HPP__
#define __GPUDB_INGESTOR_HPP__


#include <map>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>


namespace gpudb
{

// Forward declaration
class GPUdb;



class WorkerList
{
private:

    std::vector<std::string> m_worker_urls;

    static std::vector<std::string>& split_string( const std::string &in_string,
                                                   char delim,
                                                   std::vector<std::string> &elements );

public:

    WorkerList( const GPUdb &gpudb );

};  // end class WorkerList



class RecordKey
{
private:

    unsigned char *m_buffer;
    int32_t m_hash_code;
    int64_t m_routing_hash;

    RecordKey();

public:

    RecordKey( size_t buffer_size );
    ~RecordKey();

    // void add_string( std::string value );

    // void add_double( double value ):

    // void add_float( float value ):

    // void add_int8( int32_t value ):

    // void add_int16( int32_t value ):

    // void add_int( int32_t value ):

    // void add_long( int64_t value ):

    void compute_hashes();


};  // end class RecordKey




class RecordKeyBuilder
{
private:

    enum ColumnType_T
    {
        CHAR1,
        CHAR2,
        CHAR4,
        CHAR8,
        CHAR16,
        DOUBLE,
        FLOAT,
        INT,
        INT8,
        INT16,
        LONG,
        STRING
    };

    std::vector<int32_t>  m_pk_shard_key_indices;
    std::vector<ColumnType_T>  m_column_types;
    size_t m_key_buffer_size;

    RecordKeyBuilder() {}

public:




};  // end class RecordKeyBuilder



/*
 *  The WorkerQueue class maintains queues of record to be inserted
 *  into GPUdb.  It is templated on the type of the record that is to
 *  be ingested into the DB server.
 */
template<typename T>
class WorkerQueue
{
private:

    std::string     m_url;
    size_t          m_capacity;
    bool            m_has_primary_key;
    bool            m_update_on_existing_pk;
    std::vector<T>  m_queue;
    /// Map of the primary key to the key's index in the record queue
    typedef std::map<RecordKey, size_t> primary_key_map_t;
    primary_key_map_t  m_primary_key_map;

    WorkerQueue();

public:

    WorkerQueue( std::string url, size_t capacity, bool has_primary_key,
                 bool update_on_existing_pk );

    /// Returns the current queue and creates a new internal queue
    std::vector<T>& flush();

    /// Inserts a record into the queue
    std::vector<T>& insert( T record, RecordKey key );


}; // end class WorkerQueue




template<typename T>
class GPUdbIngestor : private boost::noncopyable
{

public:

    T x;

    GPUdbIngestor() { x = 0; }




};  // end class GPUdbIngestor



} // namespace gpudb



#endif // __GPUDB_INGESTOR_HPP__


