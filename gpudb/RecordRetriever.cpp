#include "gpudb/RecordRetriever.hpp"

#include "gpudb/utils/GPUdbMultiHeadIOUtils.h"




namespace gpudb
{

RecordRetriever::RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                                  const std::string& table_name ) :
    m_db( db ),
    m_record_type( record_type )
{
    WorkerList worker_list( db );
    construct( db, record_type, table_name, worker_list );
}  // end RecordRetriever constructor


RecordRetriever::RecordRetriever( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                                  const std::string& table_name,
                                  const WorkerList& worker_list ) :
    m_db( db ),
    m_record_type( record_type )
{
    construct( db, record_type, table_name, worker_list );
}  // end RecordRetriever constructor



void RecordRetriever::construct( const gpudb::GPUdb& db,
                               const gpudb::Type& record_type,
                               const std::string& table_name,
                               const WorkerList& worker_list )
{
    m_table_name  = table_name;

    // Set up the shard key builder
    // ----------------------------
    m_shard_key_builder_ptr   = new RecordKeyBuilder( false, record_type );

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
    } catch (GPUdbException e)
    {
        throw GPUdbException( e.what() );
    } catch (std::exception e)
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

        // Create the options map for the /get/records call
        std::map<std::string, std::string> options;
        options[ gpudb::get_records_by_column_expression ] = full_expression;
        options[ gpudb::get_records_fast_index_lookup    ] = gpudb::get_records_true;

        // Create a /get/records request packet
        GetRecordsRequest request( m_table_name, 0, gpudb::GPUdb::END_OF_SET, options );

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
    } catch (GPUdbException e)
    {
        throw GPUdbException( e.what() );
    } catch (std::exception e)
    {
        throw GPUdbException( e.what() );
    }
}  // end getRecordsByKey


} // end namespace gpudb
















