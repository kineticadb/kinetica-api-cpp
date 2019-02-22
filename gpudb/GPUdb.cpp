#include "gpudb/GPUdb.hpp"

#include "gpudb/utils/Utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <snappy.h>

#include <algorithm>  // for std::random_shuffle
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>  // for std::out_of_range
#include <vector>


namespace gpudb {
    static const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

        #ifndef GPUDB_NO_HTTPS
        m_sslContext( options.getSslContext() ),
        #endif

        m_username( options.getUsername() ),
        m_password( options.getPassword() ),
        m_authorization( ( !options.getUsername().empty() || !options.getPassword().empty() )
                        ? base64Encode(options.getUsername() + ":" + options.getPassword() )
                        : ""),
        m_useSnappy( options.getUseSnappy() ),
        m_threadCount( options.getThreadCount() ),
        m_executor( options.getExecutor() ),
        m_httpHeaders( options.getHttpHeaders() ),
        m_timeout( options.getTimeout() ),
        m_options( options )
    {
        // Head node URLs
        m_urls.push_back(url);

        init();
    }

    GPUdb::GPUdb(const std::string& url, const Options& options) :

        #ifndef GPUDB_NO_HTTPS
        m_sslContext(options.getSslContext()),
        #endif

        m_username(options.getUsername()),
        m_password(options.getPassword()),
        m_authorization((!options.getUsername().empty() || !options.getPassword().empty())
                        ? base64Encode(options.getUsername() + ":" + options.getPassword())
                        : ""),
        m_useSnappy(options.getUseSnappy()),
        m_threadCount(options.getThreadCount()),
        m_executor(options.getExecutor()),
        m_httpHeaders(options.getHttpHeaders()),
        m_timeout(options.getTimeout()),
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

        #ifndef GPUDB_NO_HTTPS
        m_sslContext(options.getSslContext()),
        #endif

        m_username(options.getUsername()),
        m_password(options.getPassword()),
        m_authorization((!options.getUsername().empty() || !options.getPassword().empty())
                        ? base64Encode(options.getUsername() + ":" + options.getPassword())
                        : ""),
        m_useSnappy(options.getUseSnappy()),
        m_threadCount(options.getThreadCount()),
        m_executor(options.getExecutor()),
        m_httpHeaders(options.getHttpHeaders()),
        m_timeout(options.getTimeout()),
        m_options( options )
    {
        if (urls.empty())
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }

        init();
    }

    GPUdb::GPUdb(const std::vector<std::string>& urls, const Options& options) :

        #ifndef GPUDB_NO_HTTPS
        m_sslContext(options.getSslContext()),
        #endif

        m_username(options.getUsername()),
        m_password(options.getPassword()),
        m_authorization((!options.getUsername().empty() || !options.getPassword().empty())
                        ? base64Encode(options.getUsername() + ":" + options.getPassword())
                        : ""),
        m_useSnappy(options.getUseSnappy()),
        m_threadCount(options.getThreadCount()),
        m_executor(options.getExecutor()),
        m_httpHeaders(options.getHttpHeaders()),
        m_timeout(options.getTimeout()),
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

    
    void GPUdb::init()
    {
        if ( m_urls.empty() )
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }

        // If only one URL is given, get the HA ring head node addresses, if any
        if ( m_urls.size() == 1 )
        {
            // Get the system properties to find out about the HA ring nodes
            ShowSystemPropertiesRequest  request;
            ShowSystemPropertiesResponse response;
            try {
                HttpUrl url( m_urls[0], "/show/system/properties" );
                response = submitRequest( url, request, response );
                // response = submitRequest( "/show/system/properties", request, response );
            } catch (GPUdbException ex) {
                // Note: Not worth dying just because the HA ring node
                // addresses couldn't be found
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

                        // Now save these head node addresses, including the given one
                        m_urls.clear();
                        m_urls.assign( ha_urls.begin(), ha_urls.end() );
                    }
                }
            } catch (const std::out_of_range &oor)
            {
                // Note: Not worth dying just because the appropriate properties
                // are missing (and so we can't get the HA ring node addresses)
            }
        } // end if one URL is given

        // Seed the RNG for later use
        std::srand( std::time( NULL ) );
        
        // Create host manager URLs from the regular URLs
        createHostManagerUrls( m_urls, m_options.getHostManagerPort() );

        // Make sure that there are the same number of regular URLs and HM URLs
        if ( m_urls.size() != m_hmUrls.size() )
        {
            throw std::runtime_error("Number of head node URLs and host manager URLs do not match");
        }

        // Generate a list of indices for the URL lists
        m_urlIndices.reserve( m_urls.size() );
        for ( size_t i = 0; i < m_urls.size(); ++i )
        {
            m_urlIndices.push_back( i );
        }
        // Randomly shuffle the list of indices
        std::random_shuffle( m_urlIndices.begin(), m_urlIndices.end() );

        // We'll go through this list of indices serially, having an overall
        // effect of randomly choosing the URLs
        m_currentUrl = 0;
    }   // end init

    
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

    size_t GPUdb::getThreadCount() const
    {
        return m_threadCount;
    }

    size_t GPUdb::getTimeout() const
    {
        return m_timeout;
    }

    const HttpUrl& GPUdb::getUrl() const
    {
        if (m_urls.size() == 1)
        {
            return m_urls[0];
        }
        else
        {
            boost::mutex::scoped_lock lock(m_urlMutex);

            return m_urls[ m_urlIndices[ m_currentUrl ] ];
        }
    }

    const HttpUrl* GPUdb::getUrlPointer() const
    {
        if (m_urls.size() == 1)
        {
            return &m_urls[0];
        }
        else
        {
            boost::mutex::scoped_lock lock(m_urlMutex);

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
        if (m_hmUrls.size() == 1)
        {
            return &m_hmUrls[0];
        }
        else
        {
            boost::mutex::scoped_lock lock(m_urlMutex);

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
            httpRequest.addRequestHeader("Authorization", "Basic " + m_authorization);
        }
    }


    /*
     * Create a host manager URL from a head node URL, and store it in m_hmUrls.
     */
    void GPUdb::createHostManagerUrl( const HttpUrl& url, uint16_t hostManagerPort )
    {
        HttpUrl hmUrl = HttpUrl( url.getProtocol(),
                                 url.getHost(),
                                 hostManagerPort,
                                 url.getPath(),
                                 url.getQuery() );
        m_hmUrls.push_back( hmUrl );
    }

    /*
     * Create host manager URLs from head node URLs and a given
     * host manager port number.
     */
    void GPUdb::createHostManagerUrls( const std::vector<HttpUrl>& urls,
                                       uint16_t hostManagerPort )
    {
        for( std::vector<HttpUrl>::const_iterator iter = urls.begin();
             iter != urls.end(); ++iter )
        {
            createHostManagerUrl( *iter, hostManagerPort );
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
        } catch (std::exception ex)
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
                catch (GPUdbHAUnavailableException ha_ex)
                {
                    throw GPUdbException( ha_ex.what() );
                }
            }
            catch (const GPUdbSubmitException& ex)
            {   // Some error occurred during the HTTP request
                try
                {
                    url = switchUrl( originalUrl );
                }
                catch (GPUdbHAUnavailableException ha_ex)
                {
                    throw GPUdbException( ha_ex.what() );
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
                catch (GPUdbHAUnavailableException ha_ex)
                {
                    throw GPUdbException( ha_ex.what() );
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
                catch (GPUdbHAUnavailableException ha_ex)
                {
                    throw GPUdbException( ha_ex.what() );
                }
            }
            catch (const GPUdbSubmitException& ex)
            {   // Some error occurred during the HTTP request
                try
                {
                    hmUrl = switchHmUrl( originalHmUrl );
                }
                catch (GPUdbHAUnavailableException ha_ex)
                {
                    throw GPUdbException( ha_ex.what() );
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
                catch (GPUdbHAUnavailableException ha_ex)
                {
                    throw GPUdbException( ha_ex.what() );
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
                initHttpRequest(httpRequest);
                std::string compressedRequest;
                snappy::Compress((char*)&request[0], request.size(), &compressedRequest);
                httpRequest.addRequestHeader("Content-type", "application/x-snappy");
                httpRequest.addRequestHeader("Content-length", boost::lexical_cast<std::string>(compressedRequest.length()));
                httpRequest.setRequestBody(&compressedRequest);
                httpRequest.send(httpResponse);
            }
            else
            {
                BinaryHttpRequest httpRequest(url);
                initHttpRequest(httpRequest);
                httpRequest.addRequestHeader("Content-type", "application/octet-stream");
                httpRequest.addRequestHeader("Content-length", boost::lexical_cast<std::string>(request.size()));
                httpRequest.setRequestBody(&request);
                httpRequest.send(httpResponse);
            }

            if (httpResponse.getResponseCode() == 401)
            {
                throw GPUdbException("Insufficient credentials");
            }

            avro::decode(response, httpResponse.getResponseBody());
        }
        catch (const std::exception& ex )
        {
            throw GPUdbSubmitException( url, request, ex.what() );
        }

        if (throwOnError && response.status == "ERROR")
        {
            // If Kinetica is exiting, throw a special exception
            if ( response.message.find( "Kinetica is exiting" ) != std::string::npos )
            {
                throw GPUdbExitException(response.message);
            }
            throw GPUdbException( response.message );
        }
    }   // end submitRequestRaw( HttpUrl )


    const HttpUrl* GPUdb::switchUrl(const HttpUrl* oldUrl) const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);

        // If we have just one URL, then we can't switch to another!
        if ( m_urls.size() == 1 )
        {
            throw GPUdbHAUnavailableException( "GPUdb unavailable (no HA ring available); ", m_urls );
        }
        // Increment the index by one (mod url list length)
        m_currentUrl = (m_currentUrl + 1) % m_urls.size();
        
        // If we've circled back; shuffle the indices again so that future
        // requests go to a different randomly selected cluster, but also
        // let the caller know that we've circled back
        if (&m_urls[ m_urlIndices[ m_currentUrl ] ] == oldUrl)
        {
            // Re-shuffle and set the index counter to zero
            std::random_shuffle( m_urlIndices.begin(), m_urlIndices.end() );
            // Set the index counter to zero
            m_currentUrl = 0;

            // Let the user know that we've circled back
            throw GPUdbHAUnavailableException( "GPUdb unavailable (an HA ring has been set up); ", m_urls );
        }

        // Haven't circled back to the old URL; so return the new one
        return &m_urls[ m_urlIndices[ m_currentUrl ] ];
    }


    const HttpUrl* GPUdb::switchHmUrl(const HttpUrl* oldUrl) const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);

        // If we have just one URL, then we can't switch to another!
        if ( m_hmUrls.size() == 1 )
            throw GPUdbHAUnavailableException( "GPUdb unavailable (no HA ring available); ", m_hmUrls );
        // Increment the index by one (mod url list length)
        m_currentUrl = (m_currentUrl + 1) % m_hmUrls.size();
        
        // If we've circled back; shuffle the indices again so that future
        // requests go to a different randomly selected cluster, but also
        // let the caller know that we've circled back
        if (&m_hmUrls[ m_urlIndices[ m_currentUrl ] ] == oldUrl)
        {
            // Re-shuffle and set the index counter to zero
            std::random_shuffle( m_urlIndices.begin(), m_urlIndices.end() );
            // Set the index counter to zero
            m_currentUrl = 0;

            // Let the user know that we've circled back
            throw GPUdbHAUnavailableException( "GPUdb unavailable (an HA ring has been set up); ", m_hmUrls );
        }

        // Haven't circled back to the old URL; so return the new one
        return &m_hmUrls[ m_urlIndices[ m_currentUrl ] ];
    }

    GPUdb::Options::Options() :
        #ifndef GPUDB_NO_HTTPS
        m_sslContext( NULL ),
        #endif

        m_useSnappy( true ),
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

    #ifndef GPUDB_NO_HTTPS
    boost::asio::ssl::context* GPUdb::Options::getSslContext() const
    {
        return m_sslContext;
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

    #ifndef GPUDB_NO_HTTPS
    GPUdb::Options& GPUdb::Options::setSslContext(boost::asio::ssl::context* value)
    {
        m_sslContext = value;
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

#include "gpudb/GPUdbFunctions.cpp"
}
