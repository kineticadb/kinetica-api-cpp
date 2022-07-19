#ifndef __GPUDB_MULTIHEAD_IO_UTILS_H__
#define __GPUDB_MULTIHEAD_IO_UTILS_H__

#include "gpudb/GPUdb.hpp"
#include "gpudb/Http.hpp"
#include "gpudb/Type.hpp"

#include <regex>



namespace gpudb
{

// Forward declaration
class GPUdb;


/*
 * A list of worker URLs to use for multi-head ingest.
 */
class WorkerList
{
private:

    typedef std::vector<gpudb::HttpUrl> worker_list;


public:

    /* Creates a <see cref="WorkerList"/> object and automatically populates it with the
     * worker URLs from GPUdb to support multi-head ingest. ( If the
     * specified GPUdb instance has multi-head ingest disabled, the worker
     * list will be empty and multi-head ingest will not be used.) Note that
     * in some cases, workers may be configured to use more than one IP
     * address, not all of which may be accessible to the client; this
     * constructor uses the first IP returned by the server for each worker.
     *
     * If multi-head ingestion is turned off, then returns the server's head
     * node address.
     * </summary>
     *
     * <param name="db">The <see cref="GPUdb"/> instance from which to
     * obtain the worker URLs.</param>
     */
    WorkerList( const GPUdb &gpudb );

    /* Creates a <see cref="WorkerList"/> object and automatically populates it with the
     * worker URLs from GPUdb to support multi-head ingest. ( If the
     * specified GPUdb instance has multi-head ingest disabled, the worker
     * list will be empty and multi-head ingest will not be used.) Note that
     * in some cases, workers may be configured to use more than one IP
     * address, not all of which may be accessible to the client; this
     * constructor uses the provided regular expression to match the workers in each
     * group, and only uses matching workers, if any.
     *
     * If multi-head ingestion is turned off, then returns the server's head
     * node address.
     * </summary>
     *
     * <param name="db">The <see cref="GPUdb"/> instance from which to
     * obtain the worker URLs.</param>
     * <param name="ip_regex_str">A regular expression pattern for the IPs to match.</param>
     */
    WorkerList( const GPUdb &gpudb, const std::string& ip_regex_str );

//    ~WorkerList();


    // Return the size of this WorkerList
    size_t size() const { return m_worker_urls.size(); }

    // Iterator related stuff
    typedef worker_list::const_iterator const_iterator;
    const_iterator begin() const { return m_worker_urls.begin(); }
    const_iterator end()   const { return m_worker_urls.end();   }

    // Returns if this WorkerList is empty
    bool empty() const { return m_worker_urls.empty(); }

    // Returns a string representation of the workers contained within
    std::string toString() const;


private:

    worker_list m_worker_urls;

    static void split_string( const std::string &in_string,
                              char delim,
                              std::vector<std::string> &elements );
};  // end class WorkerList



/*
 * A key based on a given record that serves as either a primary key
 * or a shard key.  The <see cref="RecordKeyBuilder"/> class creates
 * these record keys.
 */
class RecordKey
{
public:

    RecordKey();
    RecordKey( size_t buffer_size );
    RecordKey( const RecordKey &other );
    ~RecordKey();

    // Returns whether the key is valid at the moment
    bool is_valid() const { return m_is_valid; }

    // Return the key's hash code
    int32_t get_hash_code() const { return m_hash_code; }
    
    // Return the key's routing hash code
    int64_t get_routing_hash() const { return m_routing_hash; }

    // Resets the key to be an empty one with the new buffer size
    void reset( size_t buffer_size );


    // Adds a char1 to the buffer
    void add_char1( const std::string& value, bool is_null );

    // Adds a char2 to the buffer
    void add_char2( const std::string& value, bool is_null );

    // Adds a char4 to the buffer
    void add_char4( const std::string& value, bool is_null );

    // Adds a char8 to the buffer
    void add_char8( const std::string& value, bool is_null );

    // Adds a char16 to the buffer
    void add_char16( const std::string& value, bool is_null );

    // Adds a char32 to the buffer
    void add_char32( const std::string& value, bool is_null );

    // Adds a char64 to the buffer
    void add_char64( const std::string& value, bool is_null );

    // Adds a char128 to the buffer
    void add_char128( const std::string& value, bool is_null );

    // Adds a char256 to the buffer
    void add_char256( const std::string& value, bool is_null );

    // Adds a date to the buffer
    void add_date( const std::string& value, bool is_null );

    // Adds a datetime to the buffer
    void add_datetime( const std::string& value, bool is_null );

    // Adds a decimal to the buffer
    void add_decimal( const std::string& value, bool is_null );

    // Adds a double to the buffer
    void add_double( double value, bool is_null );

    // Adds a float to the buffer
    void add_float( float value, bool is_null );

    // Adds an int8 to the buffer
    void add_int8( int8_t value, bool is_null );

    // Adds an int16 to the buffer
    void add_int16( int16_t value, bool is_null );

    // Adds an integer to the buffer
    void add_int( int32_t value, bool is_null );

    // Adds a IPv4 address to the buffer
    void add_ipv4( const std::string& value, bool is_null );

    // Adds a long to the buffer
    void add_long( int64_t value, bool is_null );

    // Adds a time to the buffer
    void add_time( const std::string& value, bool is_null );

    // Adds a timestamp (long) to the buffer
    void add_timestamp( int64_t value, bool is_null );

    // Adds (the hash value of) a string to the buffer
    void add_string( const std::string& value, bool is_null );

    // Adds an unsigned long value to the buffer
    void add_ulong( const std::string& value, bool is_null );


    /// Compute the hash of the key in the buffer
    void compute_hash();

    /// Given a routing table consisting of worker rank indices, choose a
    /// worker rank based on the hash of the record key.
    size_t route( const std::vector<int32_t>& routing_table ) const;

    /// A static utility function for verifying if a given string is a valid
    // ulong value
    static bool verify_ulong_value( const std::string& value );
    
    /// The assignment operator
    RecordKey& operator=(const RecordKey& other);

    /// Returns true if the other RecordKey is equivalent to this key
    bool operator==(const RecordKey& rhs) const;
    bool operator!=(const RecordKey& rhs) const {  return !(*this == rhs); }

    /// Returns true if this RecordKey is less than the other key
    bool operator<(const RecordKey& rhs) const;
    bool operator>(const RecordKey& rhs) const {  return ( !(*this < rhs)
                                                           && !(*this == rhs) ); }

    std::string toString( const std::string& separator = " " ) const;
private:

    // Copy contents of another key into this one
    void copy( const RecordKey& other );

    // Returns whether the buffer is full or not
    bool is_buffer_full( bool throw_if_full = true ) const;

    // Check whether the buffer will overflow if we attempt to add n more bytes
    bool will_buffer_overflow( int n, bool throw_if_overflow = true ) const;

    // Adds a single byte to the buffer; does the accounting, too
    void add( uint8_t b );

    std::vector<unsigned char> m_buffer;
    size_t  m_buffer_size;
    size_t  m_current_size;
    int32_t m_hash_code;
    int64_t m_routing_hash;
    bool    m_is_valid;
    bool    m_key_is_complete;

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
        CHAR32,
        CHAR64,
        CHAR128,
        CHAR256,
        DATE,
        DATETIME,
        DECIMAL,
        DOUBLE,
        FLOAT,
        INT,
        INT8,
        INT16,
        IPV4,
        LONG,
        STRING,
        TIME,
        TIMESTAMP,
        ULONG
    };

    // Some typedefs for nullable types
    typedef boost::optional<int32_t> nullableInt;

    gpudb::Type                m_record_type;
    std::vector<int32_t>       m_pk_shard_key_indices;
    std::vector<ColumnType_T>  m_column_types;
    size_t                     m_key_buffer_size;

    RecordKeyBuilder() : m_record_type( gpudb::Type( "empty_type" ) ) {}

public:

    // Constructs a RecordKey builder
    RecordKeyBuilder( bool is_primary_key, const gpudb::Type& record_type );

    // Build a RecordKey object based on a generic record
    bool build( const gpudb::GenericRecord& record, RecordKey& record_key ) const;

    /*
     * Build a key-lookup expression based on a generic record.
     *
     * Returns true if expression building succeeded, false otherwise.
     */
    bool buildExpression( const gpudb::GenericRecord& record,
                          std::string& result ) const;

    // Returns whether this builder builds any routing keys. That is,
    // if there are any routing columns in the relevant record type
    bool has_key() const { return !m_pk_shard_key_indices.empty(); }

    // Returns true if the other RecordKeyBuilder is equivalent to this builder
    bool operator==(const RecordKeyBuilder& rhs) const;

    bool operator!=(const RecordKeyBuilder& rhs) const {  return !(*this == rhs); }
};  // end class RecordKeyBuilder



/*
 *  The WorkerQueue class maintains queues of record to be inserted
 *  into GPUdb.  It is templated on the type of the record that is to
 *  be ingested into the DB server.
 */
class WorkerQueue
{
public:

    // We need a shared pointer to move vectors of records around
    typedef std::vector<gpudb::GenericRecord>  recordVector_T;

private:

    gpudb::HttpUrl   m_url;
    size_t           m_capacity;
    recordVector_T   m_queue;

    WorkerQueue();

public:

    // Takes a string for the url; capacity 1, no PK, no update on existing PK
    // (this is used by the RecordRetriever class which doesn't care about
    // the other arguments)
    WorkerQueue( const std::string& url );

    // Takes a string for the url
    WorkerQueue( const std::string& url, size_t capacity );
    ~WorkerQueue();

    /// Returns the URL in string format for this worker
    const gpudb::HttpUrl& get_url() const { return m_url; }

    /// Returns the current queue and creates a new internal queue
    void flush( recordVector_T& flushed_records );

    /// Inserts a record into the queue
    bool insert( const gpudb::GenericRecord& record,
                 const RecordKey& key,
                 recordVector_T& flushed_records );


}; // end class WorkerQueue





} // namespace gpudb



#endif // __GPUDB_MULTIHEAD_IO_UTILS_H__


