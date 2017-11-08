#ifndef __GPUDB_INGESTOR_HPP__
#define __GPUDB_INGESTOR_HPP__

#include "gpudb/GPUdb.hpp"
#include "gpudb/Http.hpp"
#include "gpudb/Type.hpp"


#include <atomic>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


namespace gpudb
{

// Forward declaration
class GPUdb;

void test_worker_list( std::string host );
void test_record_key();
void test_record_key_builder();
void test_worker_queue();


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



// Internal classes
class RecordKeyBuilder;
class WorkerQueue;


/*
 * The multi-head ingestor class (also handles regular insertion).  Using this class is
 * significantly more computation-intensive compared to a regular insertion.  So, it is
 * highly recommended to use this ingestor only if multi-head ingestion is actually turned
 * on in the server and there is a large volume of records to be inserted.
 */
class GPUdbIngestor : private boost::noncopyable
{

public:

    GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                   const std::string& table_name,
                   const WorkerList& worker_list,
                   const std::map<std::string, std::string>& insert_options,
                   size_t batch_size );

    GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                   const std::string& table_name,
                   const WorkerList& worker_list,
                   size_t batch_size );


    GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                   const std::string& table_name,
                   const std::map<std::string, std::string>& insert_options,
                   size_t batch_size );

    GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                   const std::string& table_name, size_t batch_size );

    ~GPUdbIngestor();

    /*
     * Returns the count of records inserted so far through this ingestor
     * instance;  An atomic operation.
     */
    size_t getCountInserted() const { return m_count_inserted; }

    /*
     * Returns the count of records updated so far through this ingestor
     * instance;  An atomic operation.
     */
    size_t getCountUpdated() const { return m_count_updated; }


    /*
     * Ensures that all queued records are inserted into the database.  If an error
     * occurs while inserting the records from any queue, the recoreds will no
     * longer be in that queue nor in the database; catch <see cref="GPUdbInsertException{T}" />
     * to get the list of records that were being inserted if needed (for example,
     * to retry).  Other queues may also still contain unflushed records if this
     * occurs.
     */
    void flush();


    /*
     * Queues a record for insertion into GPUdb.  If the queue reaches
     * the <member cref="batch_size" />, all records in the queue will be
     * inserted into Kinetica before the method returns.  If an error occurs
     * while inserting the records, the records will no longer be in the queue
     * nor in Kinetica; catch <see cref="InsertException{T}"/>  to get the list
     * of records that were being inserted if needed (for example, to retry).
     *
     * <param name="record">The record to insert.</param>
     */
    void insert( gpudb::GenericRecord record );


    /*
     * Queues a list of records for insertion into Kientica.  If any queue reaches
     * the <member cref="batch_size" />, all records in the queue will be
     * inserted into Kinetica before the method returns.  If an error occurs
     * while inserting the records, the records will no longer be in the queue
     * nor in Kinetica; catch <see cref="InsertException{T}"/>  to get the list
     * of records that were being inserted if needed (for example, to retry).
     *
     * <param name="records">The records to insert.</param>
     */
    void insert( std::vector<gpudb::GenericRecord> records );


private:

    typedef std::map<std::string, std::string>          str_to_str_map_t;
    typedef boost::shared_ptr<gpudb::WorkerQueue>       worker_queue_ptr_t;
//    typedef boost::shared_ptr<gpudb::RecordKeyBuilder>  record_key_buildter_ptr;


    GPUdbIngestor();

    void construct( const gpudb::GPUdb& db,
                    const gpudb::Type& record_type,
                    const std::string& table_name,
                    const WorkerList& worker_list,
                    size_t batch_size );

    /*
     * Insert the given list of records to the database residing at the given URL.
     * Upon any error, thrown InsertException with the queue of records passed into it.
     */
    void flush( const std::vector<gpudb::GenericRecord>& queue,
                const gpudb::HttpUrl& url );

    const gpudb::GPUdb&              m_db;
    std::string                      m_table_name;
    size_t                           m_batch_size;
    std::atomic<size_t>              m_count_inserted;
    std::atomic<size_t>              m_count_updated;
    str_to_str_map_t                 m_insert_options;
    gpudb::Type                      m_record_type;
    gpudb::RecordKeyBuilder*         m_primary_key_builder_ptr;
    gpudb::RecordKeyBuilder*         m_shard_key_builder_ptr;
//    record_key_buildter_ptr          m_primary_key_builder_ptr;
//    record_key_buildter_ptr          m_shard_key_builder_ptr;
    std::vector<int32_t>             m_routing_table;
    std::vector<worker_queue_ptr_t>  m_worker_queues;

};  // end class GPUdbIngestor



} // namespace gpudb



#endif // __GPUDB_INGESTOR_HPP__


