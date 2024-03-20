#include "gpudb/GPUdb.hpp"

#include "gpudb/GenericRecord.hpp"
#include "gpudb/utils/Utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <snappy.h>

#include <algorithm>  // for std::random_shuffle, std::iter_swap
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>  // for std::out_of_range
#include <vector>


namespace gpudb {
    static const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /// Special error messages indicating that a connection failure happened
    /// (generally should trigger a high-availability failover if applicable)
    const std::string GPUdb::DB_CONNECTION_RESET_ERROR_MESSAGE   = "Connection reset";
    const std::string GPUdb::DB_CONNECTION_REFUSED_ERROR_MESSAGE = "Connection refused";
    const std::string GPUdb::DB_EXITING_ERROR_MESSAGE            = "Kinetica is exiting";
    const std::string GPUdb::DB_OFFLINE_ERROR_MESSAGE            = "Kinetica is offline";
    const std::string GPUdb::DB_SYSTEM_LIMITED_ERROR_MESSAGE     = "system-limited-fatal";
    const std::string GPUdb::DB_HM_OFFLINE_ERROR_MESSAGE         = "System is offline";

    const std::string GPUdb::FAILOVER_TRIGGER_MESSAGES[] = {
        DB_CONNECTION_RESET_ERROR_MESSAGE,
        DB_CONNECTION_REFUSED_ERROR_MESSAGE,
        DB_OFFLINE_ERROR_MESSAGE,
        DB_EXITING_ERROR_MESSAGE,
        DB_SYSTEM_LIMITED_ERROR_MESSAGE,
        DB_HM_OFFLINE_ERROR_MESSAGE
    };

    const size_t GPUdb::NUM_TRIGGER_MESSAGES = sizeof(FAILOVER_TRIGGER_MESSAGES)/sizeof(FAILOVER_TRIGGER_MESSAGES[0]); 

    /// Headers used internally; MUST add each of them to PROTECTED_HEADERS
    /// in the .cpp file
    const std::string GPUdb::HEADER_AUTHORIZATION  = "Authorization";
    const std::string GPUdb::HEADER_CONTENT_TYPE   = "Content-type";
    const std::string GPUdb::HEADER_CONTENT_LENGTH = "Content-length";
    const std::string GPUdb::HEADER_HA_SYNC_MODE   = "X-Kinetica-Group";

    const std::string GPUdb::PROTECTED_HEADERS[] = { HEADER_AUTHORIZATION,
                                                     HEADER_CONTENT_TYPE,
                                                     HEADER_CONTENT_LENGTH,
                                                     HEADER_HA_SYNC_MODE
    };

    const std::string GPUdb::HA_SYNCHRONICITY_MODE_VALUES[] = {
        "default",
        "REPL_NONE",
        "REPL_SYNC",
        "REPL_ASYNC"
    };
    
    std::string base64Encode(const std::string& value)
    {
        size_t padding = value.length() % 3;
        padding = (padding > 0) ? 3 - padding : padding;
        size_t blocks = (value.length() + padding) / 3;
        std::string result;
        result.reserve(blocks * 4);

        for (size_t i = 0; i < blocks; ++i)
        {
            size_t start = i * 3;
            char c0 = (start < value.length()) ? value[start] : 0;
            char c1 = (start + 1 < value.length()) ? value[start + 1] : 0;
            char c2 = (start + 2 < value.length()) ? value[start + 2] : 0;
            result.push_back(base64Chars[((c0 & 0xfc) >> 2)]);
            result.push_back(base64Chars[((c0 & 0x03) << 4) + ((c1 & 0xf0) >> 4)]);
            result.push_back(base64Chars[((c1 & 0x0f) << 2) + ((c2 & 0xc0) >> 6)]);
            result.push_back(base64Chars[((c2 & 0x3f) << 0)]);
        }

        for (size_t i = result.length() - padding; i < result.length(); ++i)
        {
            result[i] = '=';
        }

        return result;
    }

    size_t randomItem(size_t max)
    {
        boost::mt19937 engine;
        boost::uniform_int<size_t> generator(0, max - 1);
        engine.seed((uint32_t)std::time(0));
        return generator(engine);
    }

    GPUdb::GPUdb(const HttpUrl& url, const Options& options) :
        m_primaryUrlPtr(NULL),

        #ifndef GPUDB_NO_HTTPS
        m_sslContext( options.getSslContext() ),
        #endif

        m_username( options.getUsername() ),
        m_password( options.getPassword() ),
        m_authorization( ( !options.getUsername().empty() || !options.getPassword().empty() )
                        ? base64Encode(options.getUsername() + ":" + options.getPassword() )
                        : ""),
        m_useSnappy( options.getUseSnappy() ),
        m_disableFailover( options.getDisableFailover() ),

        #ifndef GPUDB_NO_HTTPS

        m_bypassSslCertCheck( options.getBypassSslCertCheck() ),

        #endif

        m_disableAutoDiscovery( options.getDisableAutoDiscovery() ),
        m_threadCount( options.getThreadCount() ),
        m_executor( options.getExecutor() ),
        m_httpHeaders( options.getHttpHeaders() ),
        m_timeout( options.getTimeout() ),
        m_haSyncMode( HASynchronicityMode::DEFAULT ),
        m_options( options )
    {
        // Head node URLs
        m_urls.push_back(url);

        init();
    }

    GPUdb::GPUdb(const std::string& url, const Options& options) :
        m_primaryUrlPtr(NULL),

        #ifndef GPUDB_NO_HTTPS
        m_sslContext(options.getSslContext()),
        #endif

        m_username(options.getUsername()),
        m_password(options.getPassword()),
        m_authorization((!options.getUsername().empty() || !options.getPassword().empty())
                        ? base64Encode(options.getUsername() + ":" + options.getPassword())
                        : ""),
        m_useSnappy(options.getUseSnappy()),
        m_disableFailover( options.getDisableFailover() ),

        #ifndef GPUDB_NO_HTTPS

        m_bypassSslCertCheck( options.getBypassSslCertCheck() ),

        #endif

        m_disableAutoDiscovery( options.getDisableAutoDiscovery() ),
        m_threadCount(options.getThreadCount()),
        m_executor(options.getExecutor()),
        m_httpHeaders(options.getHttpHeaders()),
        m_timeout(options.getTimeout()),
        m_haSyncMode( HASynchronicityMode::DEFAULT ),
        m_options( options )
    {
        // Split on commas, if any
        char comma = ',';
        std::size_t comma_index = url.find( comma );
        if ( comma_index != std::string::npos )
        {
            // Multiple URLs found; parse them all URL
            std::istringstream iss( url );
            std::string token;
            while ( std::getline( iss, token, comma ) )
            {
                // Parse a single URL
                HttpUrl url_ = HttpUrl( token );
                m_urls.push_back( url_ );
            }
        }
        else
        {
            // Parse single URL
            HttpUrl url_ = HttpUrl( url );
            m_urls.push_back( url_ );
        }

        init();
    }

    GPUdb::GPUdb(const std::vector<HttpUrl>& urls, const Options& options) :
        m_urls(urls),
        m_primaryUrlPtr(NULL),

        #ifndef GPUDB_NO_HTTPS
        m_sslContext(options.getSslContext()),
        #endif

        m_username(options.getUsername()),
        m_password(options.getPassword()),
        m_authorization((!options.getUsername().empty() || !options.getPassword().empty())
                        ? base64Encode(options.getUsername() + ":" + options.getPassword())
                        : ""),
        m_useSnappy(options.getUseSnappy()),
        m_disableFailover( options.getDisableFailover() ),

        #ifndef GPUDB_NO_HTTPS

        m_bypassSslCertCheck( options.getBypassSslCertCheck() ),

        #endif

        m_disableAutoDiscovery( options.getDisableAutoDiscovery() ),
        m_threadCount(options.getThreadCount()),
        m_executor(options.getExecutor()),
        m_httpHeaders(options.getHttpHeaders()),
        m_timeout(options.getTimeout()),
        m_haSyncMode( HASynchronicityMode::DEFAULT ),
        m_options( options )
    {
        if (urls.empty())
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }

        init();
    }

    GPUdb::GPUdb(const std::vector<std::string>& urls, const Options& options) :
        m_primaryUrlPtr(NULL),

        #ifndef GPUDB_NO_HTTPS
        m_sslContext(options.getSslContext()),
        #endif

        m_username(options.getUsername()),
        m_password(options.getPassword()),
        m_authorization((!options.getUsername().empty() || !options.getPassword().empty())
                        ? base64Encode(options.getUsername() + ":" + options.getPassword())
                        : ""),
        m_useSnappy(options.getUseSnappy()),
        m_disableFailover( options.getDisableFailover() ),

        #ifndef GPUDB_NO_HTTPS

        m_bypassSslCertCheck( options.getBypassSslCertCheck() ),

        #endif

        m_disableAutoDiscovery( options.getDisableAutoDiscovery() ),
        m_threadCount(options.getThreadCount()),
        m_executor(options.getExecutor()),
        m_httpHeaders(options.getHttpHeaders()),
        m_timeout(options.getTimeout()),
        m_haSyncMode( HASynchronicityMode::DEFAULT ),
        m_options( options )
    {
        if (urls.empty())
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }

        m_urls.reserve(urls.size());
        m_urls.insert(m_urls.end(), urls.begin(), urls.end());

        init();
    }


    GPUdb::~GPUdb()
    {
        // Release resources
        if ( m_primaryUrlPtr != NULL )
        {
            delete m_primaryUrlPtr;
        }
    }
    
    
    void GPUdb::init()
    {
        if ( m_urls.empty() )
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }

        // Seed the RNG for later use
        std::srand( std::time( NULL ) );

        // Handle the primary host URL, if any is given
        m_primaryUrlStr = m_options.getPrimaryUrl();
        handlePrimaryURL();

        // Get the HA ring head node addresses, if any
        if( !m_disableAutoDiscovery )
        {
            // Get the HA ring head node addresses, if any
            getHAringHeadNodeAddresses();
        }

        // Create host manager URLs from the regular URLs
        updateHostManagerUrls();

        // Randomly shuffle the URL list handler (not the list of URLs
        // itself, but another list which keeps the former's indices)
        randomizeURLs();

        // Ensure that the default synchronicity mode is set in the headers
        setHASyncMode( m_haSyncMode );
    }   // end init


    /**
     *  Handle the primary host URL, if any is given via options
     */
    void GPUdb::handlePrimaryURL()
    {
        // No-op if an empty string is given
        if ( m_primaryUrlStr.empty() ) {

            if ( m_urls.size() == 1 )
            {
                // Use the one URL provided by the user as the primary,
                // but only if the user has not specified any primary URL
                // also
                m_primaryUrlStr = m_urls[ 0 ].getUrl();
            }
            else
            {   // No primary URL to handle;
                // release resources, if necessary
                if (m_primaryUrlPtr != NULL)
                {
                    delete m_primaryUrlPtr;
                }
                // Need a null to know we don't have any primary cluster
                m_primaryUrlPtr = NULL;
                return;
            }
        }

        // Make sure that if we've already handled the primary cluster URL, we're not
        // repeating the work
        if ( m_primaryUrlPtr != NULL )
        {
            if ( m_primaryUrlStr.compare( (std::string)*m_primaryUrlPtr ) == 0 )
            {
                // The primary cluster URL has already been processed
                return;
            }
            else
            {
                // The saved URL doesn't match; we need to process it properly;
                // so, release resources
                delete m_primaryUrlPtr;
            }
        }
        
        // Parse the URL
        try {
            // Need to remember to release resources in the destructor
            m_primaryUrlPtr = new HttpUrl( m_primaryUrlStr );
        } catch (const std::invalid_argument& ex) {
            std::string message = GPUDB_STREAM_TO_STRING( "Error parsing the primary host URL: "
                                                          << ex.what() );
            throw new GPUdbException( message );
        }

        {   // Make the URL handling thread-safe
            boost::mutex::scoped_lock lock(m_urlMutex);
            
            // Check if the primary host exists in the list of user given hosts
            std::vector<HttpUrl>::iterator primaryUrlIter;
            primaryUrlIter = std::find( m_urls.begin(), m_urls.end(), *m_primaryUrlPtr );
            if ( primaryUrlIter != m_urls.end() ) {
                // The primary URL already exists in the list
                if ( primaryUrlIter != m_urls.begin() ) {
                    // Note: Do not combine the nested if with the top level if; will change
                    //       logic and may end up getting duplicates of the primary URL

                    // Move the primary URL to the front of the list
                    std::iter_swap( primaryUrlIter, m_urls.begin() );
                }
            } else {
                // The primary URL does not exist in the list and must be the
                // first element in the list
                m_urls.insert( m_urls.begin(), *m_primaryUrlPtr );
            }
        }

        // Update the host manager URLs
        updateHostManagerUrls();
        return;
    }   // end handlePrimaryUrl


    /**
     * Update the URLs with the available HA ring information
     */
    void GPUdb::getHAringHeadNodeAddresses()
    {
        // Get the system properties to find out about the HA ring nodes
        ShowSystemPropertiesRequest  request;
        ShowSystemPropertiesResponse response;
        try {
            HttpUrl url( m_urls[0], "/show/system/properties" );
            response = submitRequest( url, request, response );
        } catch (const GPUdbException&) {
            // Note: Not worth dying just because the HA ring node
            // addresses couldn't be found
            return;
        }

        const std::map<std::string, std::string> &systemProperties = response.propertyMap;
        try
        {
            std::string is_ha_enabled = systemProperties.at( gpudb::show_system_properties_conf_enable_ha );

            // Only attempt to parse the HA ring node addresses if HA is enabled
            if ( is_ha_enabled.compare( gpudb::show_system_properties_TRUE ) == 0 )
            {
                // Parse the HA ringt head node addresses, if any
                std::string ha_ring_head_nodes = systemProperties.at( gpudb::show_system_properties_conf_ha_ring_head_nodes );

                // Parse the ring head node addresses, if any
                if ( !ha_ring_head_nodes.empty() )
                {
                    std::vector<HttpUrl> ha_urls;
                    try
                    {
                        // Split on commas, if any
                        char comma = ',';
                        std::size_t comma_index = ha_ring_head_nodes.find( comma );
                        if ( comma_index != std::string::npos )
                        {
                            // Multiple URLs found; parse them all URL
                            std::istringstream iss( ha_ring_head_nodes );
                            std::string token;
                            while ( std::getline( iss, token, comma ) )
                            {
                                // Parse a single URL
                                HttpUrl url_ = HttpUrl( token );
                                ha_urls.push_back( url_ );
                            }
                        }
                        else
                        {
                            // Parse single URL
                            HttpUrl url_ = HttpUrl( ha_ring_head_nodes );
                            ha_urls.push_back( url_ );
                        }
                    } catch ( const std::invalid_argument& ex )
                    {
                        std::string message = GPUDB_STREAM_TO_STRING( "Error parsing HA ring head "
                                                                      << "node address from the "
                                                                      << "database configuration "
                                                                      << "parameters: "
                                                                      << ex.what() );
                        throw GPUdbException( message );
                    }

                    // Ensure that the given URL is included in the HA ring
                    // head node addresses
                    std::vector<HttpUrl>::iterator it = std::find( ha_urls.begin(),
                                                                   ha_urls.end(),
                                                                   m_urls[0] );
                    if ( it == ha_urls.end() )
                    {   // not found; so add the given URL to the list
                        ha_urls.push_back( m_urls[ 0 ] );
                    }

                    {
                        // This operation needs to be thread-safe
                        boost::mutex::scoped_lock lock(m_urlMutex);

                        // Now save these head node addresses, including the given one
                        m_urls.clear();
                        m_urls.assign( ha_urls.begin(), ha_urls.end() );
                    }

                    // Handle the primary host URL, if any is given
                    handlePrimaryURL();

                    // Re-create host manager URLs from the regular URLs
                    updateHostManagerUrls();
                    
                    // Randomly shuffle the URL list handler (not the list of URLs
                    // itself, but another list which keeps the former's indices)
                    randomizeURLs();
                }   // end if ha ring non-empty
            }   // end if ha is enabled
        } catch (const std::out_of_range &oor)
        {
            // Note: Not worth dying just because the appropriate properties
            // are missing (and so we can't get the HA ring node addresses)
        }
    }   // end getHAringHeadNodeAdresses


    /**
     * Randomly shuffles the list of high availability URL indices so that HA
     * failover happens at a random fashion.  One caveat is when a primary host
     * is given by the user; in that case, we need to keep the primary host's
     * index as the first one in the list so that upon failover, when we cricle
     * back, we always pick the first/primary host up again.
     */
    void GPUdb::randomizeURLs() const
    {
        // This operation needs to be thread-safe
        boost::mutex::scoped_lock lock(m_urlMutex);

        // Re-create the list of HA URL indices
        // Generate a list of indices for the URL lists
        m_urlIndices.clear();
        for ( size_t i = 0; i < m_urls.size(); ++i )
        {
            m_urlIndices.push_back( i );
        }

        // Randomly shuffle the list of indices
        if ( m_primaryUrlPtr == NULL )
        {
            // We don't have any primary URL; so treat all URLs similarly
            // Randomly order the HA clusters and pick one to start working with
            std::random_shuffle( m_urlIndices.begin(), m_urlIndices.end() );
        } else
        {
            // Shuffle from the 2nd element onward, only if there are more than
            // two elements, of course
            if ( m_urlIndices.size() > 2 ) {
                std::random_shuffle( m_urlIndices.begin() + 1,
                                     m_urlIndices.end() );
            }
        }

        // This will keep track of which cluster to pick next (an index of
        // randomly shuffled indices)
        m_currentUrl = 0;
    }   // end randomizeURLs


    /**
     * Convert a given high availability synchronicity override mode enumeration
     * to its string value.
     */
    const std::string& GPUdb::getHASynchronicityModeValue( HASynchronicityMode syncMode ) const
    {
        return HA_SYNCHRONICITY_MODE_VALUES[ syncMode ];
    }


    void GPUdb::addKnownType(const std::string& typeId, const avro::DecoderPtr& decoder)
    {
        if (!decoder)
        {
            boost::mutex::scoped_lock lock(m_knownTypesMutex);
            m_knownTypes.erase(typeId);
        }
        else
        {
            boost::mutex::scoped_lock lock(m_knownTypesMutex);
            m_knownTypes[typeId] = decoder;
        }
    }

    void GPUdb::addKnownTypeFromTable(const std::string& tableName, const avro::DecoderPtr& decoder)
    {
        ShowTableRequest request(tableName, std::map<std::string, std::string>());
        ShowTableResponse response;
        submitRequest("/show/table", request, response);

        if (response.typeIds.empty())
        {
            throw GPUdbException("Table " + tableName + " does not exist.");
        }

        std::string& typeId = response.typeIds[0];

        for (std::vector<std::string>::const_iterator it = response.typeIds.begin();
             it != response.typeIds.end(); ++it)
        {
            if (*it != typeId)
            {
                throw GPUdbException("Table " + tableName + " is not homogeneous.");
            }
        }

        if (!decoder)
        {
            boost::mutex::scoped_lock lock(m_knownTypesMutex);
            m_knownTypes.erase(typeId);
        }
        else
        {
            boost::mutex::scoped_lock lock(m_knownTypesMutex);
            m_knownTypes[typeId] = decoder;
        }
    }

    avro::DecoderPtr GPUdb::getDecoder(const std::string& typeId) const
    {
        {
            boost::mutex::scoped_lock lock(m_knownTypesMutex);

            if (m_knownTypes.find(typeId) != m_knownTypes.end())
            {
                return m_knownTypes[typeId];
            }
        }

        ShowTypesRequest request(typeId, "", std::map<std::string, std::string>());
        ShowTypesResponse response;
        submitRequest("/show/types", request, response);

        if (response.typeSchemas.size() == 0)
        {
            throw GPUdbException("Unable to obtain type information for type " + typeId + ".");
        }

        Type type(response.labels[0], response.typeSchemas[0], response.properties[0]);
        avro::DecoderPtr decoder = avro::createDecoder<GenericRecord>(type);
        boost::mutex::scoped_lock lock(m_knownTypesMutex);

        if (m_knownTypes.find(typeId) != m_knownTypes.end())
        {
            return m_knownTypes[typeId];
        }

        m_knownTypes[typeId] = decoder;
        return decoder;
    }

    avro::ExecutorPtr GPUdb::getExecutor() const
    {
        return m_executor;
    }

    const std::map<std::string, std::string>& GPUdb::getHttpHeaders() const
    {
        return m_httpHeaders;
    }

    const std::string& GPUdb::getPassword() const
    {
        return m_password;
    }

    const std::string& GPUdb::getPrimaryURL() const
    {
        return m_primaryUrlStr;
    }

    size_t GPUdb::getThreadCount() const
    {
        return m_threadCount;
    }

    GPUdb::HASynchronicityMode GPUdb::getHASyncMode() const
    {
        return m_haSyncMode;
    }

    size_t GPUdb::getTimeout() const
    {
        return m_timeout;
    }

    const HttpUrl& GPUdb::getUrl() const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);
        if (m_urls.size() == 1)
        {
            return m_urls[0];
        }
        else
        {
            return m_urls[ m_urlIndices[ m_currentUrl ] ];
        }
    }

    const HttpUrl* GPUdb::getUrlPointer() const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);
        if (m_urls.size() == 1)
        {
            return &m_urls[0];
        }
        else
        {
            return &m_urls[ m_urlIndices[ m_currentUrl ] ];
        }
    }

    const std::vector<HttpUrl>& GPUdb::getUrls() const
    {
        return m_urls;
    }

    const HttpUrl& GPUdb::getHmUrl() const
    {
        if (m_hmUrls.size() == 1)
        {
            return m_hmUrls[0];
        }
        else
        {
            boost::mutex::scoped_lock lock( m_urlMutex );

            return m_hmUrls[ m_urlIndices[ m_currentUrl ] ];
        }
    }

    const HttpUrl* GPUdb::getHmUrlPointer() const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);
        if (m_hmUrls.size() == 1)
        {
            return &m_hmUrls[0];
        }
        else
        {
            return &m_hmUrls[ m_urlIndices[ m_currentUrl ] ];
        }
    }

    const std::vector<HttpUrl>& GPUdb::getHmUrls() const
    {
        return m_hmUrls;
    }

    bool GPUdb::getUseSnappy() const
    {
        return m_useSnappy;
    }

    const std::string& GPUdb::getUsername() const
    {
        return m_username;
    }

    #ifndef GPUDB_NO_HTTPS

    boost::asio::ssl::context* GPUdb::getSslContext() const
    {
        return m_sslContext;
    }

    bool GPUdb::getBypassSslCertCheck() const
    {
        return m_bypassSslCertCheck;
    }

    #endif

    long GPUdb::execute(const std::string& sql)
    {
        std::string parameters;
        return execute(sql, parameters);
    }

    long GPUdb::execute(const std::string& sql, const std::string& parameters)
    {
        std::map<std::string, std::string> options;
        return execute(sql, parameters, options);
    }

    long GPUdb::execute(const std::string& sql, const std::string& parameters,
                        const std::map<std::string, std::string>& options)
    {
        std::map<std::string, std::string> sql_options = options; // Make a copy to add to
        if (!parameters.empty())
            sql_options[execute_sql_query_parameters] = parameters;

        std::vector<std::vector<uint8_t> > data;
        ExecuteSqlResponse response = executeSql(sql, 0, 1, "", data, sql_options);

        return response.countAffected;
    }

    template<typename T> long GPUdb::execute(const std::string& sql, const std::vector<T>& parameters)
    {
        std::map<std::string, std::string> options;
        return execute(sql, parameters, options);
    }

    template<typename T> long GPUdb::execute(const std::string& sql, const std::vector<T>& parameters,
                                             const std::map<std::string, std::string>& options)
    {
        std::string json_params = JsonStringFromVector(parameters);
        return execute(sql, json_params, options);
    }

#define DECLARE_ARRAY_TYPES(x)  \
    template long GPUdb::execute(const std::string& sql, const std::vector<x>& parameters); \
    template long GPUdb::execute(const std::string& sql, const std::vector<x>& parameters, \
                                 const std::map<std::string, std::string>& options);

    DECLARE_ARRAY_TYPES(bool)
    DECLARE_ARRAY_TYPES(int32_t)
    DECLARE_ARRAY_TYPES(int64_t)
    DECLARE_ARRAY_TYPES(float)
    DECLARE_ARRAY_TYPES(double)
    DECLARE_ARRAY_TYPES(std::string)

    /**
     * Sets the high-availability synchronization mode which will override
     * the default mode.
     */
    void GPUdb::setHASyncMode( GPUdb::HASynchronicityMode mode )
    {
        m_haSyncMode = mode;

        // Add the sync mode to the list of headers (overriding it if extant)
        m_httpHeaders[ HEADER_HA_SYNC_MODE ] = getHASynchronicityModeValue( m_haSyncMode );
    }


    /**
     * Adds an HTTP header to the map of additional HTTP headers to send to
     * GPUdb with each request. If the header is already in the map, its
     * value is replaced with the specified value.  The user is not allowed
     * to modify the following headers:
     * <ul>
     *    <li> Authorization
     *    <li> Content-type
     *    <li> Content-length
     *    <li> ha_sync_mode
     * </ul>
     *
     * @param header  the HTTP header
     * @param value   the value of the HTTP header
     *
     * See {@link #getHttpHeaders()}
     * See {@link #removeHttpHeader(const std::string&)}
     */
    void GPUdb::addHttpHeader( const std::string& header,
                               const std::string& value )
    {
        // Ensure that the given header is not a protecte header
        size_t num_protected_headers = sizeof(PROTECTED_HEADERS)/sizeof(PROTECTED_HEADERS[0]);
        for ( size_t i = 0; i < num_protected_headers; ++i )
        {
            if ( header == PROTECTED_HEADERS[ i ] )
            {
                throw GPUdbException( "Not allowed to change proteced header: "
                                      + header );
            }
        }
        
        m_httpHeaders[ header ] = value;
        return;
    }

    /**
     * Removes the given HTTP header from the map of additional HTTP headers to
     * send to GPUdb with each request. The user is not allowed to remove the
     * following headers:
     * <ul>
     *    <li> Authorization
     *    <li> Content-type
     *    <li> Content-length
     *    <li> ha_sync_mode
     * </ul>
     *
     * @param header  the HTTP header
     *
     * See {@link #getHttpHeaders()}
     * See {@link #addHttpHeader(const std::string&, const std::string&)}
     */
    void GPUdb::removeHttpHeader( const std::string& header )
    {
        // Ensure that the given header is not a protecte header
        size_t num_protected_headers = sizeof(PROTECTED_HEADERS)/sizeof(PROTECTED_HEADERS[0]);
        for ( size_t i = 0; i < num_protected_headers; ++i )
        {
            if ( header == PROTECTED_HEADERS[ i ] )
            {
                throw GPUdbException( "Not allowed to remove proteced header: "
                                      + header );
            }
        }
        
        m_httpHeaders.erase( header );
        return;
    }


    
    void GPUdb::initHttpRequest(HttpRequest& httpRequest) const
    {
        #ifndef GPUDB_NO_HTTPS
        httpRequest.setSslContext(m_sslContext);
        #endif

        httpRequest.setTimeout(m_timeout);
        httpRequest.setRequestMethod(HttpRequest::POST);

        for (std::map<std::string, std::string>::const_iterator it = m_httpHeaders.begin();
             it != m_httpHeaders.end(); ++it)
        {
            httpRequest.addRequestHeader(it->first, it->second);
        }

        if (!m_authorization.empty())
        {
            httpRequest.addRequestHeader( HEADER_AUTHORIZATION,
                                          "Basic " + m_authorization );
        }
    }



    /*
     * Create host manager URLs from head node URLs and a given
     * host manager port number.
     */
    void GPUdb::updateHostManagerUrls()
    {
        // Get the host manager port
        uint16_t hostManagerPort = m_options.getHostManagerPort();

        // The URL handling has to be thread-safe
        boost::mutex::scoped_lock lock(m_urlMutex);

        // Recreate the host manager URL list
        m_hmUrls.clear();
        
        for( std::vector<HttpUrl>::const_iterator iter = m_urls.begin();
             iter != m_urls.end(); ++iter )
        {
            try
            {
                HttpUrl hmUrl = HttpUrl( iter->getProtocol(),
                                         iter->getHost(),
                                         hostManagerPort,
                                         iter->getPath(),
                                         iter->getQuery() );
                m_hmUrls.push_back( hmUrl );
            } catch ( const std::invalid_argument& ex )
            {
                throw new GPUdbException( ex.what() );
            }
        }
        
        // Make sure that there are the same number of regular URLs and HM URLs
        if ( m_urls.size() != m_hmUrls.size() )
        {
            throw GPUdbException("Number of head node URLs and host manager URLs do not match");
        }
    }
    
    /**
     * Re-sets the host manager port number for the host manager URLs. Some
     * endpoints are supported only at the host manager, rather than the
     * head node of the database.
     *
     * @param[in] value  the host manager port number
     */
    void GPUdb::setHostManagerPort(uint16_t value)
    {
        // The URL handling has to be thread-safe
        boost::mutex::scoped_lock lock(m_urlMutex);

        // Reset the port for all the host manager URLs
        for ( size_t i = 0; i < m_hmUrls.size(); ++i )
        {
            try
            {
                HttpUrl oldUrl = m_hmUrls.at( i );
                // Change only the port to be the host manager port
                HttpUrl newUrl = HttpUrl( oldUrl.getProtocol(),
                                          oldUrl.getHost(),
                                          value,
                                          oldUrl.getPath(),
                                          oldUrl.getQuery() );
                m_hmUrls[ i ] = newUrl;
            } catch ( const std::invalid_argument& ex )
            {
                throw new GPUdbException( ex.what() );
            }
        }
    }

    /**
     * Automatically resets the host manager port number for the host manager
     * URLs by finding out what the host manager port is.
     */
    void GPUdb::updateHostManagerPort()
    {
        // Find out from the database server what the correct port
        // number is
        ShowSystemPropertiesRequest request;
        ShowSystemPropertiesResponse response;
        submitRequest("/show/system/properties", request, response );
        const std::string& port_str = response.propertyMap[ gpudb::show_system_properties_conf_hm_http_port ];

        if ( port_str.empty() )
        {
            throw new GPUdbException("Missing value for '"
                                     + show_system_properties_conf_hm_http_port
                                     + "'" );
        }

        
        // Parse the integer value from the string
        uint16_t hmPort;
        try
        {
            std::istringstream iss( port_str );
            iss >> hmPort;
        } catch (const std::exception&)
        {
            throw new GPUdbException( "No parsable value found for host manager port number." );
            // throw new GPUdbException( "No parsable value found for host manager port number "
            //                           + "(expected integer, given '" + port_str + "')" );
        }

        // Update the host manager URLs with the correct port number
        setHostManagerPort( hmPort );
        return;
    }


    void GPUdb::setDecoderIfMissing(const std::string& typeId,
                                    const std::string& label,
                                    const std::string& schemaString,
                                    const std::map<std::string, std::vector<std::string> >& properties) const
    {
        // If the table is a collection, it does not have a proper type so
        // ignore it

        if (typeId == "<collection>")
        {
            return;
        }

        {
            boost::mutex::scoped_lock lock(m_knownTypesMutex);

            if (m_knownTypes.find(typeId) != m_knownTypes.end())
            {
                return;
            }
        }

        Type type(label, schemaString, properties);
        avro::DecoderPtr decoder = avro::createDecoder<GenericRecord>(type);
        boost::mutex::scoped_lock lock(m_knownTypesMutex);

        if (m_knownTypes.find(typeId) != m_knownTypes.end())
        {
            return;
        }

        m_knownTypes[typeId] = decoder;
    }


    void GPUdb::submitRequestRaw(const std::string& endpoint,
                                 const std::vector<uint8_t>& request,
                                 RawGpudbResponse& response,
                                 const bool enableCompression) const
    {
        const HttpUrl* url = getUrlPointer();
        const HttpUrl* originalUrl = url;

        while (true)
        {
            try
            {
                submitRequestRaw(HttpUrl(*url, endpoint), request, response, enableCompression, true);
                break;
            }
            catch (const GPUdbExitException& ex)
            {   // First handle our special exit exception
                try
                {
                    url = switchUrl( originalUrl );
                }
                catch (const GPUdbHAUnavailableException& ha_ex)
                {
                    std::string message = GPUDB_STREAM_TO_STRING( ha_ex.what()
                                                                  << "; original exception: "
                                                                  << ex.what() );
                    throw GPUdbException( message );
                }
            }
            catch (const GPUdbSubmitException& ex)
            {   // Some error occurred during the HTTP request
                try
                {
                    url = switchUrl( originalUrl );
                }
                catch (const GPUdbHAUnavailableException& ha_ex)
                {
                    std::string message = GPUDB_STREAM_TO_STRING( ha_ex.what()
                                                                  << "; original exception: "
                                                                  << ex.what() );
                    throw GPUdbException( message );
                }
            }
            catch (const GPUdbException& ex)
            {   // Any other GPUdbException is a valid failure
                throw ex;
            }
            catch (const std::exception& ex)
            {   // And other random exceptions probably are also connection errors
                try
                {
                    url = switchUrl( originalUrl );
                }
                catch (const GPUdbHAUnavailableException& ha_ex)
                {
                    std::string message = GPUDB_STREAM_TO_STRING( ha_ex.what()
                                                                  << "; original exception: "
                                                                  << ex.what() );
                    throw GPUdbException( message );
                }
            }
        }

        if (response.status == "ERROR")
        {   // Should'nt really get here; but just in case we do
            throw GPUdbException(response.message);
        }
    }   // end submitRequestRaw( string endpoint )

    
    void GPUdb::submitRequestToHostManagerRaw(const std::string& endpoint,
                                              const std::vector<uint8_t>& request,
                                              RawGpudbResponse& response,
                                              const bool enableCompression) const
    {
        const HttpUrl* hmUrl = getHmUrlPointer();
        const HttpUrl* originalHmUrl = hmUrl;

        while (true)
        {
            try
            {
                submitRequestRaw(HttpUrl(*hmUrl, endpoint), request, response, enableCompression, true);
                break;
            }
            catch (const GPUdbExitException& ex)
            {   // First handle our special exit exception
                try
                {
                    hmUrl = switchHmUrl( originalHmUrl );
                }
                catch (const GPUdbHAUnavailableException& ha_ex)
                {
                    std::string message = GPUDB_STREAM_TO_STRING( ha_ex.what()
                                                                  << "; original exception: "
                                                                  << ex.what() );
                    throw GPUdbException( message );
                }
            }
            catch (const GPUdbSubmitException& ex)
            {   // Some error occurred during the HTTP request
                try
                {
                    hmUrl = switchHmUrl( originalHmUrl );
                }
                catch (const GPUdbHAUnavailableException& ha_ex)
                {
                    std::string message = GPUDB_STREAM_TO_STRING( ha_ex.what()
                                                                  << "; original exception: "
                                                                  << ex.what() );
                    throw GPUdbException( message );
                }
            }
            catch (const GPUdbException& ex)
            {   // Any other GPUdbException is a valid failure
                throw ex;
            }
            catch (const std::exception& ex)
            {   // And other random exceptions probably are also connection errors
                try
                {
                    hmUrl = switchHmUrl( originalHmUrl );
                }
                catch (const GPUdbHAUnavailableException& ha_ex)
                {
                    std::string message = GPUDB_STREAM_TO_STRING( ha_ex.what()
                                                                  << "; original exception: "
                                                                  << ex.what() );
                    throw GPUdbException( message );
                }
            }  // end try-catch
        }  // end while

        if (response.status == "ERROR")
        {   // Should'nt really get here; but just in case we do
            throw GPUdbException( response.message );
        }
    }   // end submitRequestToHostManagerRaw( endpoint )


    void GPUdb::submitRequestRaw(const HttpUrl& url,
                                 const std::vector<uint8_t>& request,
                                 RawGpudbResponse& response,
                                 const bool enableCompression,
                                 const bool throwOnError) const
    {
        BinaryHttpResponse httpResponse;

        try
        {
            if (enableCompression && m_useSnappy)
            {
                StringHttpRequest httpRequest(url);
                httpRequest.setBypassSslCertCheck(this->getBypassSslCertCheck());
                initHttpRequest(httpRequest);
                std::string compressedRequest;
                snappy::Compress((char*)&request[0], request.size(), &compressedRequest);
                httpRequest.addRequestHeader( HEADER_CONTENT_TYPE, "application/x-snappy" );
                httpRequest.addRequestHeader( HEADER_CONTENT_LENGTH,
                                              boost::lexical_cast<std::string>(compressedRequest.length()) );
                httpRequest.setRequestBody(&compressedRequest);
                httpRequest.send(httpResponse);
            }
            else
            {
                BinaryHttpRequest httpRequest(url);
                httpRequest.setBypassSslCertCheck(this->getBypassSslCertCheck());
                initHttpRequest(httpRequest);
                httpRequest.addRequestHeader( HEADER_CONTENT_TYPE, "application/octet-stream" );
                httpRequest.addRequestHeader( HEADER_CONTENT_LENGTH,
                                              boost::lexical_cast<std::string>(request.size()) );
                httpRequest.setRequestBody(&request);
                httpRequest.send(httpResponse);
            }

            if (httpResponse.getResponseCode() == 401)
            {
                throw GPUdbException("Insufficient credentials");
            }
            else if ((httpResponse.getResponseCode() >= 300) &&
                     (httpResponse.getResponseCode() != 400)) // GPUdb errors returned as 400
            {
                throw GPUdbException(std::to_string(httpResponse.getResponseCode()) + ": " +
                    httpResponse.getResponseMessage());
            }

            avro::decode(response, httpResponse.getResponseBody());
        }
        catch (const std::exception& ex )
        {
            std::string error_msg = ex.what();

            // If the error message contains any of the HA failover trigger messages,
            // then return an exit exception to indicate that we need to do an HA failover
            for ( size_t i = 0; i < NUM_TRIGGER_MESSAGES; ++i )
            {
                if ( error_msg.find( FAILOVER_TRIGGER_MESSAGES[ i ] ) != std::string::npos )
                {
                    throw GPUdbExitException( error_msg );
                }
            }

            // For any other error, throw a submit exception
            throw GPUdbSubmitException( url, request, error_msg );
        }

        if (throwOnError && response.status == "ERROR")
        {
            // If the error message contains any of the HA failover trigger messages,
            // then return an exit exception to indicate that we need to do an HA failover
            for ( size_t i = 0; i < NUM_TRIGGER_MESSAGES; ++i )
            {
                if ( response.message.find( FAILOVER_TRIGGER_MESSAGES[ i ] ) != std::string::npos )
                {
                    throw GPUdbExitException( response.message );
                }
            }
            throw GPUdbException( response.message );
        }
    }   // end submitRequestRaw( HttpUrl )


    const HttpUrl* GPUdb::switchUrl(const HttpUrl* oldUrl) const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);

        if( m_disableFailover ) {
            // ToDo : Log that failover has been disabled by client
            throw new GPUdbException("Failover has been disabled by the client API");
        }

        // If we have just one URL, then we can't switch to another!
        if ( m_urls.size() == 1 )
        {
            throw GPUdbHAUnavailableException( "GPUdb unavailable (no backup cluster exists); ", m_urls );
        }
        // Increment the index by one (mod url list length)
        m_currentUrl = (m_currentUrl + 1) % m_urls.size();
        
        // If we've circled back; shuffle the indices again so that future
        // requests go to a different randomly selected cluster, but also
        // let the caller know that we've circled back
        if (&m_urls[ m_urlIndices[ m_currentUrl ] ] == oldUrl)
        {
            // Release the lock since randomizeURLs() uses it, too
            lock.unlock();
            // Re-shuffle and set the index counter to zero
            randomizeURLs();

            // Let the user know that we've circled back
            throw GPUdbHAUnavailableException( "GPUdb unavailable (backup clusters exist, but all failed); ", m_urls );
        }

        // Haven't circled back to the old URL; so return the new one
        return &m_urls[ m_urlIndices[ m_currentUrl ] ];
    }


    const HttpUrl* GPUdb::switchHmUrl(const HttpUrl* oldUrl) const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);

        // If we have just one URL, then we can't switch to another!
        if ( m_hmUrls.size() == 1 )
            throw GPUdbHAUnavailableException( "GPUdb unavailable (no backup cluster exists); ", m_hmUrls );
        // Increment the index by one (mod url list length)
        m_currentUrl = (m_currentUrl + 1) % m_hmUrls.size();
        
        // If we've circled back; shuffle the indices again so that future
        // requests go to a different randomly selected cluster, but also
        // let the caller know that we've circled back
        if (&m_hmUrls[ m_urlIndices[ m_currentUrl ] ] == oldUrl)
        {
            // Release the lock since randomizeURLs() uses it, too
            lock.unlock();
            // Re-shuffle and set the index counter to zero
            randomizeURLs();

            // Let the user know that we've circled back
            throw GPUdbHAUnavailableException( "GPUdb unavailable (backup clusters exist, but all failed); ", m_hmUrls );
        }

        // Haven't circled back to the old URL; so return the new one
        return &m_hmUrls[ m_urlIndices[ m_currentUrl ] ];
    }



    // ------------------ GPUdb::Options Methods ----------------------
    
    GPUdb::Options::Options() :
        #ifndef GPUDB_NO_HTTPS
        m_sslContext( NULL ),
        m_bypassSslCertCheck( false ),
        #endif

        m_useSnappy( true ),
        m_disableFailover( false ),
        m_disableAutoDiscovery( false ),
        m_threadCount( 1 ),
        m_timeout( 0 ),
        m_hmPort( 9300 )
    {
    }

    GPUdb::Options& GPUdb::Options::addHttpHeader(const std::string& header, const std::string& value)
    {
        m_httpHeaders[header] = value;
        return *this;
    }

    avro::ExecutorPtr GPUdb::Options::getExecutor() const
    {
        return m_executor;
    }

    std::map<std::string, std::string>& GPUdb::Options::getHttpHeaders()
    {
        return m_httpHeaders;
    }

    const std::map<std::string, std::string>& GPUdb::Options::getHttpHeaders() const
    {
        return m_httpHeaders;
    }

    std::string GPUdb::Options::getPassword() const
    {
        return m_password;
    }

    std::string GPUdb::Options::getPrimaryUrl() const
    {
        return m_primaryUrl;
    }

    #ifndef GPUDB_NO_HTTPS
    boost::asio::ssl::context* GPUdb::Options::getSslContext() const
    {
        return m_sslContext;
    }
    bool GPUdb::Options::getBypassSslCertCheck() const
    {
        return m_bypassSslCertCheck;

    }
    #endif

    size_t GPUdb::Options::getThreadCount() const
    {
        return m_threadCount;
    }

    size_t GPUdb::Options::getTimeout() const
    {
        return m_timeout;
    }

    uint16_t GPUdb::Options::getHostManagerPort() const
    {
        return m_hmPort;
    }

    bool GPUdb::Options::getUseSnappy() const
    {
        return m_useSnappy;
    }

    bool GPUdb::Options::getDisableFailover() const
    {
        return m_disableFailover;
    }

    bool GPUdb::Options::getDisableAutoDiscovery() const
    {
        return m_disableAutoDiscovery;
    }

    std::string GPUdb::Options::getUsername() const
    {
        return m_username;
    }

    GPUdb::Options& GPUdb::Options::setExecutor(const avro::ExecutorPtr value)
    {
        m_executor = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setHttpHeaders(const std::map<std::string, std::string>& value)
    {
        m_httpHeaders = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setPassword(const std::string& value)
    {
        m_password = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setPrimaryUrl(const std::string& value)
    {
        m_primaryUrl = value;
        return *this;
    }

    #ifndef GPUDB_NO_HTTPS
    GPUdb::Options& GPUdb::Options::setSslContext(boost::asio::ssl::context* value)
    {
        m_sslContext = value;
        return *this;
    }
    
    GPUdb::Options& GPUdb::Options::setBypassSslCertCheck(const bool value)
    {
        m_bypassSslCertCheck = value;
        return *this;
    }
    #endif

    GPUdb::Options& GPUdb::Options::setThreadCount(const size_t value)
    {
        if (value == 0)
        {
            throw std::invalid_argument("Thread count must be greater than zero.");
        }

        m_threadCount = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setTimeout(const size_t value)
    {
        m_timeout = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setHostManagerPort(const uint16_t value)
    {
        m_hmPort = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setUseSnappy(const bool value)
    {
        m_useSnappy = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setUsername(const std::string& value)
    {
        m_username = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setDisableFailover(const bool value)
    {
        m_disableFailover = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setDisableAutoDiscovery(const bool value)
    {
        m_disableAutoDiscovery = value;
        return *this;
    }

#include "gpudb/GPUdbFunctions.cpp"
}
