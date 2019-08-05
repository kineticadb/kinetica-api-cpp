#ifndef __RECORD_RETRIEVER_HPP__
#define __RECORD_RETRIEVER_HPP__

#include "gpudb/GPUdb.hpp"
#include "gpudb/Http.hpp"
#include "gpudb/Type.hpp"
#include "gpudb/utils/GPUdbMultiHeadIOUtils.h"


#include <atomic>
#include <map>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


namespace gpudb
{

// Forward declaration
class GPUdb;


/**
 * The multi-head record retriever class.  Using this class is
 * significantly more computation-intensive compared to a regular insertion.  So, it is
 * highly recommended to use this ingestor only if multi-head ingestion is actually turned
 * on in the server and there is a large volume of records to be inserted.
 *
 * Also, for now, only key-lookup is available via this class, meaning, the
 * user has to supply the values for all the shard columns via the
 * get_records_by_key() method; also, this method does not support unsharded
 * tables.
 */
class RecordRetriever : private boost::noncopyable
{

public:

    RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                     const std::string& table_name,
                     const WorkerList& worker_list );

    RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                     const std::string& table_name );

    RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                     const std::string& table_name,
                     const WorkerList& worker_list,
                     const std::map<std::string, std::string>& retrieval_options );

    RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                     const std::string& table_name,
                     const std::map<std::string, std::string>& retrieval_options );


    ~RecordRetriever();

    /**
     * Returns the name of the table on which this class operates.
     */
    const std::string& getTableName() const { return m_table_name; }

    /**
     * Returns the GPUdb client handle that this class uses internally.
     */
    const gpudb::GPUdb& getGPUdb() const { return m_db; }


    /**
     * Gets the options currently used for the retriever methods.  Note
     * that any gpudb::get_records_by_column_expression option will
     * get overridden at the next {@link #getRecordsByKey} call with the
     * appropriate expression.
     *
     */
    const std::map<std::string, std::string>& getOptions() const { return m_retrieval_options; }


    /**
     * Returns the options currently used for the retriever methods.  Note
     * that any gpudb::get_records_by_column_expression option will
     * be overridden at the next {@link #getRecordsByKey} call with the
     * appropriate expression.
     */
    void setOptions( const std::map<std::string, std::string>& options );


    /**
     * Retrieves records for a given shard key, optionally further limited by an
     * additional expression. All records matching the key and satisfying the
     * expression will be returned, up to the system-defined limit. For
     * multi-head mode the request will be sent directly to the appropriate
     * worker.
     * <p>
     * All fields in both the shard key and the expression must have defined
     * attribute indexes, unless the shard key is also a primary key and all
     * referenced fields are in the primary key. The expression must be limited
     * to basic equality and inequality comparisons that can be evaluated using
     * the attribute indexes.
     *
     * @param[in] key_values  A vector of shard column values
     * @param[in] expression  An optional expression.  Default is an empty string.
     *
     * @return
     */
    GetRecordsResponse<gpudb::GenericRecord> getRecordsByKey( const gpudb::GenericRecord& record,
                                                              const std::string& expression = "" );


private:

    typedef std::map<std::string, std::string>      str_to_str_map_t;
    typedef boost::shared_ptr<gpudb::WorkerQueue>   worker_queue_ptr_t;


    RecordRetriever();

    void construct( const gpudb::GPUdb& db,
                    const gpudb::Type& record_type,
                    const std::string& table_name,
                    const WorkerList& worker_list,
                    const std::map<std::string, std::string>& retrieval_options );

    const gpudb::GPUdb&              m_db;
    std::string                      m_table_name;
    gpudb::Type                      m_record_type;
    gpudb::RecordKeyBuilder*         m_shard_key_builder_ptr;
    std::vector<int32_t>             m_routing_table;
    std::vector<worker_queue_ptr_t>  m_worker_queues;
    str_to_str_map_t                 m_retrieval_options;

};  // end class RecordRetriever



} // namespace gpudb



#endif // __RECORD_RETRIEVER_HPP__


