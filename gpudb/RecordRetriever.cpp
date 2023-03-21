#include "gpudb/RecordRetriever.hpp"

#include "gpudb/utils/GPUdbMultiHeadIOUtils.h"




namespace gpudb
{

RecordRetriever::RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                                  const std::string& table_name ) :
    m_db( db ),
    m_record_type( record_type )
{
    // Pass a blank map for the options
    std::map<std::string, std::string> retrieval_options;
    
    WorkerList worker_list( db );
    construct( db, record_type, table_name, worker_list, retrieval_options );
}  // end RecordRetriever constructor


RecordRetriever::RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                                  const std::string& table_name,
                                  const WorkerList& worker_list ) :
    m_db( db ),
    m_record_type( record_type )
{
    // Pass a blank map for the options
    std::map<std::string, std::string> retrieval_options;
    
    construct( db, record_type, table_name, worker_list, retrieval_options );
}  // end RecordRetriever constructor


RecordRetriever::RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                                  const std::string& table_name,
                                  const std::map<std::string, std::string>& retrieval_options ) :
    m_db( db ),
    m_record_type( record_type )
{

    WorkerList worker_list( db );
    construct( db, record_type, table_name, worker_list, retrieval_options );
}  // end RecordRetriever constructor


RecordRetriever::RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                                  const std::string& table_name,
                                  const WorkerList& worker_list,
                                  const std::map<std::string, std::string>& retrieval_options ) :
    m_db( db ),
    m_record_type( record_type )
{
    // Construct the remaining members
    construct( db, record_type, table_name, worker_list, retrieval_options );
}  // end RecordRetriever constructor



void RecordRetriever::construct( const gpudb::GPUdb& db,
                                 const gpudb::Type& record_type,
                                 const std::string& table_name,
                                 const WorkerList& worker_list,
                                 const std::map<std::string, std::string>& retrieval_options )
{
    m_table_name = table_name;

    // Save the options
    setOptions( retrieval_options );

    // Set up the shard key builder
    // ----------------------------
    m_shard_key_builder_ptr = new RecordKeyBuilder( false, record_type );

    // Check if there are shard keys
    if ( !m_shard_key_builder_ptr->has_key() )
    {   // release the source
        delete m_shard_key_builder_ptr;
        m_shard_key_builder_ptr = NULL;
    }

    
    // Set up the worker queues
    // ------------------------
    try
    {
        // If we have multiple workers, then use those
        if ( !worker_list.empty() )
        {
            // Add a worker (record) queue per worker rank of the database
            for ( WorkerList::const_iterator it = worker_list.begin(); it != worker_list.end(); ++it )
            {
                std::string url = (std::string)( it->getAppendedUrl( "get/records" ) );
                worker_queue_ptr_t worker_queue( new WorkerQueue( url ) );
                m_worker_queues.push_back( worker_queue );
            }

            // Get the worker rank information from the db server
            gpudb::AdminShowShardsResponse admin_show_shards_rsp;
            std::map<std::string, std::string> options;
            db.adminShowShards( options, admin_show_shards_rsp );
            m_routing_table.swap( admin_show_shards_rsp.rank );

            // Check that enough worker URLs are specified
            for ( size_t i = 0; i < m_routing_table.size(); ++i )
            {
                if ( m_routing_table[ i ] > (int32_t)m_worker_queues.size() )
                    throw GPUdbException( "Not enough worker URLs specified" );
            }
        }
        else // multi-head ingest is NOT turned on; use the regular server IP address
        {
            std::string url = (std::string)( m_db.getUrl().getAppendedUrl( "get/records" ) );
            worker_queue_ptr_t worker_queue( new WorkerQueue( url ) );
            m_worker_queues.push_back( worker_queue );
        }
    } catch (const GPUdbException& e)
    {
        throw GPUdbException( e.what() );
    } catch (const std::exception& e)
    {
        throw GPUdbException( e.what() );
    }

    // Initialize the random seed
    std::srand( std::time( NULL ) );

    return;
}  // end construct


RecordRetriever::~RecordRetriever()
{
    // Release resources
    delete m_shard_key_builder_ptr;

    m_worker_queues.clear();
    m_routing_table.clear();
}  // end destructor



/**
 * Returns the options currently used for the retriever methods.  Note
 * that any gpudb::get_records_by_column_expression option will
 * be overridden at the next {@link #getRecordsByKey} call with the
 * appropriate expression.
 */
void RecordRetriever::setOptions( const str_to_str_map_t& options )
{
    // Save the given options
    m_retrieval_options = options;

    // We will always need to use this for getRecordsByKey
    m_retrieval_options[ gpudb::get_records_fast_index_lookup ] = gpudb::get_records_true;
}

    

    
GetRecordsResponse<gpudb::GenericRecord>
RecordRetriever::getRecordsByKey( const gpudb::GenericRecord& record,
                                  const std::string& expression )
{
    if ( m_shard_key_builder_ptr == NULL )
        throw new GPUdbException( "Cannot get by key from unsharded table: " + m_table_name );

    try
    {
        // Build the expression
        std::string full_expression;
        if ( !m_shard_key_builder_ptr->buildExpression( record, full_expression ) )
        {
            throw new GPUdbException( "Error in creating key-lookup expression." );
        }
        // Add any given envelope expression
        if ( !expression.empty() )
        {
            full_expression = ( full_expression + " and (" + expression + ")" );
        }

        // Set the expression in the options map
        m_retrieval_options[ gpudb::get_records_by_column_expression ] = full_expression;
        // // Currently set in the constructor
        // options[ gpudb::get_records_fast_index_lookup    ] = gpudb::get_records_true;

        // Create a /get/records request packet
        GetRecordsRequest request( m_table_name, 0, gpudb::GPUdb::END_OF_SET,
                                   m_retrieval_options );

        // Create the appropriate response objects
        RawGetRecordsResponse raw_response;
        GetRecordsResponse<gpudb::GenericRecord> decoded_response;

        // Submit the /get/records request
        if ( m_routing_table.empty() )
        {   // No routing information is available; talk to rank-0
            raw_response = m_db.submitRequest("/get/records", request, raw_response);
        }
        else // Talk to the appropriate worker rank
        {
            // Find the appropriate worker rank
            RecordKey shard_key;
            m_shard_key_builder_ptr->build( record, shard_key );
            HttpUrl url = m_worker_queues[ shard_key.route( m_routing_table ) ]->get_url();
            // Make the call
            raw_response = m_db.submitRequest(url, request, raw_response);
        }

        // Set up the values of the decoded response properly
        decoded_response.tableName  = raw_response.tableName;
        decoded_response.typeName   = raw_response.typeName;
        decoded_response.typeSchema = raw_response.typeSchema;
        decoded_response.hasMoreRecords       = raw_response.hasMoreRecords;
        decoded_response.totalNumberOfRecords = raw_response.totalNumberOfRecords;

        // Decode the records
        decoded_response.data.resize( raw_response.recordsBinary.size(),
                                      gpudb::GenericRecord( m_record_type ) );
        avro::decode( &decoded_response.data[0], &raw_response.recordsBinary[0],
                      raw_response.recordsBinary.size() );

        return decoded_response;
    } catch (const GPUdbException& e)
    {
        throw GPUdbException( e.what() );
    } catch (const std::exception& e)
    {
        throw GPUdbException( e.what() );
    }
}  // end getRecordsByKey



    /**
     * Note: If a regular retriever method is implemented (other than "by key"),
     *       then some changes would need to be made to the options.  Currently,
     *       `getByKey()` sets the `expressions` option and keeps it there since
     *       it will get overridden during the next `getByKey()` call.  However,
     *       if there is a method like `getAllRecords()` from the worker ranks
     *       directly, then such a saved expression in the options would change/
     *       limit the records fetched.  In that case, the options handling would
     *       have to be modified as necessary.  Also, currently, the `fast index
     *       lookup` option is always set in the constructor; that would not
     *       necessarily apply; we would need to take care of that as well.
     *
     *       The `setOptions()` method would have to be changed as well.
     */


} // end namespace gpudb
















