#include "gpudb/GPUdbIngestor.hpp"

#include "gpudb/utils/GPUdbMultiHeadIOUtils.h"




namespace gpudb
{

GPUdbIngestor::GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                              const std::string& table_name, size_t batch_size ) :
    m_db( db ),
    m_record_type( record_type )
{
    WorkerList worker_list( db );
    construct( db, record_type, table_name, worker_list, batch_size );
}  // end GPUdbIngestor constructor



GPUdbIngestor::GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                              const std::string& table_name,
                              const std::map<std::string, std::string>& insert_options,
                              size_t batch_size ) :
    m_db( db ),
    m_insert_options( insert_options ),
    m_record_type( record_type )
{
    WorkerList worker_list( db );
    construct( db, record_type, table_name, worker_list, batch_size );
}  // end GPUdbIngestor constructor



GPUdbIngestor::GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                              const std::string& table_name,
                              const WorkerList& worker_list,
                              size_t batch_size ) :
    m_db( db ),
    m_record_type( record_type )
{
    construct( db, record_type, table_name, worker_list, batch_size );
}  // end GPUdbIngestor constructor



GPUdbIngestor::GPUdbIngestor( const gpudb::GPUdb& db, const gpudb::Type& record_type,
                              const std::string& table_name,
                              const WorkerList& worker_list,
                              const std::map<std::string, std::string>& insert_options,
                              size_t batch_size ) :
    m_db( db ),
    m_insert_options( insert_options ),
    m_record_type( record_type )
{
    construct( db, record_type, table_name, worker_list, batch_size );
}  // end GPUdbIngestor constructor



void GPUdbIngestor::construct( const gpudb::GPUdb& db,
                               const gpudb::Type& record_type,
                               const std::string& table_name,
                               const WorkerList& worker_list,
                               size_t batch_size )
{
    m_table_name  = table_name;

    // The batch size must be greater than or equal to 1
    if ( batch_size < 1 )
    {
        std::ostringstream ss;
        ss << "Batch size must be greater than one; given " << batch_size;
        throw GPUdbException( ss.str() );
    }
    m_batch_size = batch_size;

    m_count_inserted = 0;
    m_count_updated  = 0;

    // Set up the primary and shard key builders
    // -----------------------------------------
    m_primary_key_builder_ptr = new RecordKeyBuilder( true, record_type );
    m_shard_key_builder_ptr   = new RecordKeyBuilder( false, record_type );

    if ( m_primary_key_builder_ptr->has_key() )
    {   // there are primary keys for this record type
        // Now check if there is a distinct shard key
        if ( !m_shard_key_builder_ptr->has_key()
             || (*m_shard_key_builder_ptr == *m_primary_key_builder_ptr) )
        {  // No distinct shard key
            m_shard_key_builder_ptr = m_primary_key_builder_ptr;
        }
    }
    else  // there are no primary keys
    {
        // Release the source and set to null
        delete m_primary_key_builder_ptr;
        m_primary_key_builder_ptr = NULL;

        // Check if there are shard keys
        if ( !m_shard_key_builder_ptr->has_key() )
        {   // release the source
            delete m_shard_key_builder_ptr;
            m_shard_key_builder_ptr = NULL;
        }
    }

    // Set up the worker queues
    // ------------------------
    // Do we update records if there are matching primary keys in the database?
    bool update_records_on_existing_pk = false;
    str_to_str_map_t::const_iterator iter = m_insert_options.find( gpudb::insert_records_update_on_existing_pk );
    if ( iter != m_insert_options.end() )
    {   // The option is given, but is it true?
        if ( gpudb::insert_records_true.compare( iter->second ) == 0 )
            update_records_on_existing_pk = true;
    }

    // Does the record type have primary keys?
    bool has_primary_key = (m_primary_key_builder_ptr != NULL);
    try
    {
        // If we have multiple workers, then use those
        if ( !worker_list.empty() )
        {
            // Add a worker (record) queue per worker rank of the database
            for ( WorkerList::const_iterator it = worker_list.begin(); it != worker_list.end(); ++it )
            {
                std::string url = ( (std::string)it->getUrl() + "insert/records" );
                worker_queue_ptr_t worker_queue( new WorkerQueue( url, batch_size, has_primary_key,
                                                                  update_records_on_existing_pk ) );
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
            std::string url = ( (std::string)m_db.getUrl() + "insert/records" );
            worker_queue_ptr_t worker_queue( new WorkerQueue( url, batch_size, has_primary_key,
                                                              update_records_on_existing_pk ) );

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


GPUdbIngestor::~GPUdbIngestor()
{
    // Release resources
    delete m_primary_key_builder_ptr;
    delete m_shard_key_builder_ptr;

    m_worker_queues.clear();
    m_routing_table.clear();
}  // end destructor

/*
 * Ensures that all queued records are inserted into the database.  If an error
 * occurs while inserting the records from any queue, the recoreds will no
 * longer be in that queue nor in the database; catch <see cref="GPUdbInsertException{T}" />
 * to get the list of records that were being inserted if needed (for example,
 * to retry).  Other queues may also still contain unflushed records if this
 * occurs.
 */
void GPUdbIngestor::flush()
{
    std::vector<worker_queue_ptr_t>::const_iterator it;
    for ( it = m_worker_queues.begin(); it != m_worker_queues.end(); ++it )
    {
        // Flush the queue
        WorkerQueue::recordVector_T records_to_flush;
        (it->get())->flush( records_to_flush );

        // Actually insert the records
        this->flush( records_to_flush, (*it)->get_url() );
    }
}  // end public flush


/*
 * Insert the given list of records to the database residing at the given URL.
 * Upon any error, thrown InsertException with the queue of records passed into it.
 *
 * <param name="queue">The list of records to insert.</param>
 * <param name="url">The address of the database worker.</param>
 */
void GPUdbIngestor::flush( const std::vector<gpudb::GenericRecord>& queue,
                           const gpudb::HttpUrl& url )
{
    if ( queue.empty() )
    {
        return; // nothing to do since the queue is empty
    }

    try
    {
        // Encode the records into binary first
        std::vector<std::vector<uint8_t> > encoded_records;
        gpudb::avro::encode( encoded_records, queue );

        // Create the /insert/records request and response objects
        gpudb::RawInsertRecordsRequest request( m_table_name, encoded_records, m_insert_options );
        gpudb::InsertRecordsResponse response;

        // Make the /insert/records call
        m_db.submitRequest( url, request, response );

        // Save the counts of inserted and updated records
        m_count_inserted += response.countInserted;
        m_count_updated  += response.countUpdated;

    } catch (gpudb::GPUdbException e)
    {  // throw the records to the caller since they weren't inserted into the db
        throw GPUdbInsertionException( url, queue, e.what() );
    } catch (std::exception e)
    {  // throw the records to the caller since they weren't inserted into the db
        throw GPUdbInsertionException( url, queue, e.what() );
    }
}  // end private flush


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
void GPUdbIngestor::insert( gpudb::GenericRecord record )
{
    // Create the record keys
    RecordKey primary_key; // used to check for uniqueness
    RecordKey shard_key;   // used to find which worker to send this record to
    bool have_shard_key   = false;

    // Build the primary key, if any
    if ( m_primary_key_builder_ptr != NULL )
    {
        m_primary_key_builder_ptr->build( record, primary_key );
    }

    // Build the sharding/routing key, if any
    if ( m_shard_key_builder_ptr != NULL )
    {
        have_shard_key = m_shard_key_builder_ptr->build( record, shard_key );
    }

    // Find out which worker to send the record to; then add the record
    // to the appropriate worker's record queue
    worker_queue_ptr_t worker_queue_ptr;
    if ( m_routing_table.empty() )
    {   // We have no information regarding multiple worker ranks; use the
        // first/only address
        worker_queue_ptr = m_worker_queues[ 0 ];
    }
    else if ( !have_shard_key )
    {   // We do NOT have a shard key; choose a random worker
        worker_queue_ptr = m_worker_queues[ std::rand() % m_worker_queues.size() ];
    }
    else
    {   // Get the appropriate worker based on the sharding/routing key
        size_t worker_index = shard_key.route( m_routing_table );
        worker_queue_ptr = m_worker_queues[ worker_index ];
    }

    // Insert the record into the queue
    WorkerQueue::recordVector_T  flushed_records;
    worker_queue_ptr->insert( record, primary_key, flushed_records );

    // If inserting the queue resulted in flushing the queue, then flush it
    // properly
    if ( !flushed_records.empty() )
    {
        this->flush( flushed_records, worker_queue_ptr->get_url() );
    }
}  // end insert a single record




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
void GPUdbIngestor::insert( std::vector<gpudb::GenericRecord> records )
{
    // Insert one record at a time
    for ( size_t i = 0; i < records.size(); ++i )
    {
        try
        {
            this->insert( records[ i ] );
        }
        catch ( GPUdbInsertionException e )
        {
            // Add the remaining records to the insertion exception queue
            e.append_records( (records.begin() + i + 1), records.end() );

            // Rethrow
            throw e;
        }
    }  // end outer loop
}  // end insert multiple records




} // end namespace gpudb
















