#include "GPUdbIngestor.hpp"

#include "gpudb/GPUdb.hpp"

#include <cstring>
#include <map>
#include <sstream>


namespace gpudb
{

// --------------------- Implementation of Class WorkerList ----------------------

// TODO: Add an optional input parameter for a regex for URL matching
WorkerList::WorkerList( const GPUdb &gpudb )
{
    // Get the system properties information from the server
    std::map<std::string, std::string> options;
    gpudb::ShowSystemPropertiesResponse sys_properties_rsp;
    sys_properties_rsp = gpudb.showSystemProperties( options );
    std::map<std::string, std::string> &sys_property_map = sys_properties_rsp.propertyMap;

    // Check if multihead ingestion is enabled
    std::map<std::string, std::string>::iterator it;
    it = sys_property_map.find( gpudb::show_system_properties_conf_enable_worker_http_servers );
    if ( it == sys_property_map.end() )
    { // keyword not found
        throw GPUdbException( "Missing value for " + gpudb::show_system_properties_conf_enable_worker_http_servers );
    }

    // so, is multi-head ingestion enabled?
    bool multi_head_ingest_enabled = ( it->second == gpudb::show_system_properties_TRUE );
    if ( !multi_head_ingest_enabled )
    {   // nope, it is NOT enabled;
        // so, just add the server URL to the worker list and return.
        std::string gpudb_server_url = gpudb.getUrl();
        m_worker_urls.push_back( gpudb_server_url );
        return; // nothing more to do
    }

    // Multi-head ingestion IS enabled; find the worker IP addresses
    // and ports; then add them to the worker url vector.
    // -----------------------------------------------------------------

    // Get the IP addresses of the worker ranks
    std::string ip_keyword = gpudb::show_system_properties_conf_worker_http_server_ips;
    it = sys_property_map.find( ip_keyword );
    if ( it == sys_property_map.end() )
    { // keyword not found
        throw GPUdbException( "Missing value for " + ip_keyword );
    }
    // Parse the IP addresses
    char delim = ';';
    std::vector<std::string> worker_ips;
    worker_ips = split_string( it->second, delim, worker_ips );

    // Check that the there's at least one IP address given
    if ( worker_ips.empty() )
        throw GPUdbException( "Empty value for " + ip_keyword );

    // Get the port numbers of the worker ranks
    std::string port_keyword = gpudb::show_system_properties_conf_worker_http_server_ports;
    it = sys_property_map.find( port_keyword );
    if ( it == sys_property_map.end() )
    { // keyword not found
        throw GPUdbException( "Missing value for " + port_keyword );
    }
    // Parse the IP addresses
    std::vector<std::string> worker_ports;
    worker_ports = split_string( it->second, delim, worker_ports );

    // Check that the same number of IP addresses and ports are provided
    if ( worker_ips.size() != worker_ports.size() )
        throw GPUdbException( "Inconsistent number of values for " + ip_keyword + " and " + port_keyword );

    // For each worker rank, create a URL from the IP address and port number
    std::string worker_ip, worker_port;
    for ( size_t i = 0; i < worker_ips.size(); ++i )
    {
        worker_ip   = worker_ips[   i ];
        worker_port = worker_ports[ i ];

        std::string worker_url = ("http://" + worker_ip + ":" + worker_port);
        m_worker_urls.push_back( worker_url );

        // TODO: Use a regular expression to match the worker URL against it
    }

    return; // done
}  // end WorkerList constructor


// static convenience function
std::vector<std::string>& WorkerList::split_string( const std::string &in_string,
                                                    char delim,
                                                    std::vector<std::string> &elements )
{
    std::stringstream ss( in_string );
    std::string item;
    while ( std::getline( ss, item, delim ) )
    {
        elements.push_back( item );
    }
    return elements;
}  // end split_string


// --------------------- END implementation of Class WorkerList ----------------------



// --------------------- Implementation of Class RecordKey ----------------------

// Constructor
RecordKey::RecordKey( size_t buffer_size )
{
    if ( buffer_size < 1 )
        throw GPUdbException( "Too small value for input parameter 'buffer_size': "
                              + std::to_string( buffer_size ) + " (must be >= 1)" );

    // Allocate the buffer and initialize
    m_buffer = new unsigned char[ buffer_size ];
    memset( m_buffer, 0, buffer_size );
}  // end RecordKey constructor


// Destructor
RecordKey::~RecordKey()
{
    delete [] m_buffer;
}


// void RecordKey::add_double( double value )
// {
//     return;
// } // end RecordKey::add_double

// --------------------- END Implementation of Class RecordKey ----------------------




// --------------------- Implementation of Class WorkerQueue ----------------------

template<typename T>
WorkerQueue<T>::WorkerQueue( std::string url, size_t capacity,
                             bool has_primary_key,
                             bool update_on_existing_pk )
    : m_url( url ),
      m_capacity( capacity ),
      m_has_primary_key( has_primary_key ),
      m_update_on_existing_pk( update_on_existing_pk )
{
    // Reserve enough capacity for the queue
    m_queue.reserve( capacity );

    return;
}  // end WorkerQueue constructor



// template<typename T>
// WorkerQueue<T>::~WorkerQueue()
// {
//     // Free the queue
//     delete m_queue;

//     return;
// }  // end WorkerQueue destructor


/*
 *  Returns the current queue and creates a new internal queue.
 */
template<typename T>
std::vector<T>& WorkerQueue<T>::flush()
{
    // Save the current/old queue
    std::vector<T> old_queue = m_queue;

    // Create a new queue
    // (TODO: Do we need to free this guy later on?)
    m_queue = std::vector<T>();
    m_queue.reserve( m_capacity );

    return old_queue;
}  // end WorkerQueue flush()



/*
 *  Insert a record into the queue.  Returns the queue
 *  if full (post-insertion), NULL otherwise.
 */
template<typename T>
std::vector<T>& WorkerQueue<T>::insert( T record, RecordKey key )
{
    // Special insertion operation for records consisting primary keys
    if ( m_has_primary_key )
    {
        // First check if the given key already exists as a primary key
        typename primary_key_map_t::iterator it;
        it = m_primary_key_map.find( key );
        if ( it == m_primary_key_map.end() )
        { // key does NOT already exist
            // insert the record into the queue
            m_queue.push_back( record );
            
            // insert the (key, queue index) into the map that keep
            // tracks of where the record associated with a given key
            // is inside the record queue.
            m_primary_key_map[ key ] = (m_queue.size() - 1);
        }
        else
        { // key does already exist
            // Check if the table allows updates on pre-existing primary keys
            if ( m_update_on_existing_pk )
            {  // yes, it's allowed
                size_t record_index = it->second;
                m_queue[ record_index ] = record;
            }
            else
            {  // updating existing primary keys not allowed
                return NULL;
            }
        }
    }  // end if primary keys exist
    else // no primary key involved; simply add to the queue
    {
        m_queue.push_back( record );
    }

    // If the queue is full to the capacity, flush it
    if ( m_queue.size() == m_capacity )
    {
        return this->flush();
    }
    else // queue not full yet; return NULL
    {
        return NULL;
    }
}  // end WorkerQueue insert


// --------------------- END Implementation of Class WorkerQueue ----------------------



// --------------------- Implementation of Class RecordKeyBuilder ----------------------


// --------------------- END Implementation of Class RecordKeyBuilder ----------------------


} // end namespace gpudb
















