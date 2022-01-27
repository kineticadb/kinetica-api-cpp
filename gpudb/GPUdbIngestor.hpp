#ifndef __GPUDB_INGESTOR_HPP__
#define __GPUDB_INGESTOR_HPP__

#include "gpudb/GPUdb.hpp"
#include "gpudb/Http.hpp"
#include "gpudb/Type.hpp"
#include "gpudb/utils/GPUdbMultiHeadIOUtils.h"


#include <atomic>
#include <map>
#include <string>
#include <vector>
#include <mutex>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


namespace gpudb
{

// Forward declaration
class GPUdb;


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

    /**
     * Returns the name of the table on which this class operates.
     */
    const std::string& getTableName() const { return m_table_name; }

    /**
     * Returns the GPUdb client handle that this class uses internally.
     */
    const gpudb::GPUdb& getGPUdb() const { return m_db; }

    /**
     * Returns the insertion options the ingestor uses.
     */
    const std::map<std::string, std::string>& getOptions() const { return m_insert_options; }

    /**
     * Returns the count of records inserted so far through this ingestor
     * instance;  An atomic operation.
     */
    size_t getCountInserted() const { return m_count_inserted; }

    /**
     * Returns the count of records updated so far through this ingestor
     * instance;  An atomic operation.
     */
    size_t getCountUpdated() const { return m_count_updated; }

    /**
     * Returns the list of errors and warnings received since the last call
     * to getErrors(), and clears the list.
     */
    std::vector<GPUdbInsertionException> getErrors();

    /**
     * Ensures that all queued records are inserted into the database.  If an error
     * occurs while inserting the records from any queue, the recoreds will no
     * longer be in that queue nor in the database; catch <see cref="GPUdbInsertException{T}" />
     * to get the list of records that were being inserted if needed (for example,
     * to retry).  Other queues may also still contain unflushed records if this
     * occurs.
     */
    void flush();


    /**
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


    /**
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
    std::vector<GPUdbInsertionException> m_error_list;
    std::mutex                       m_error_list_lock;

};  // end class GPUdbIngestor



} // namespace gpudb



#endif // __GPUDB_INGESTOR_HPP__


