#ifndef __GPUDB_HPP__
#define __GPUDB_HPP__

#include "gpudb/protocol/EndpointKeywords.h"
#include "gpudb/utils/Utils.h"

#ifndef GPUDB_NO_HTTPS
// The practice of declaring the Bind placeholders (_1, _2, ...) in the global namespace is deprecated. Please use <boost/bind/bind.hpp> + using namespace boost::placeholders, or define BOOST_BIND_GLOBAL_PLACEHOLDERS to retain the current behavior.
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/asio/ssl.hpp>
#endif

#include <stdint.h>

#include <boost/thread/mutex.hpp>

namespace gpudb
{
    class GPUdb;
}

#include "gpudb/Avro.hpp"
#include "gpudb/GenericRecord.hpp"
#include "gpudb/GPUdbException.hpp"
#include "gpudb/Http.hpp"
#include "gpudb/Type.hpp"
#include "gpudb/protocol/GPUdbProtocol.h"


/**
 \mainpage Introduction

 This is the client-side C++ application programming interface (API) for Kinetica.

 </br>

 The source code can be found <a href="https://github.com/kineticadb/kinetica-api-cpp" target="_top">here</a>.

 </br>

 There are two projects here: gpudb and example.
 </br>

 The gpudb project contains the main client source code in the gpudb namespace.
 The <see cref="gpudb.GPUdb">GPUdb</see> class implements the interface for the API.  The protocol
 subdirectory contains classes for each endpoint of the database server.

 </br>

 The example project contains a short example in gpudb-api-example.cpp.  The user
 needs to specify the hostname of a database server (e.g. "127.0.0.1:9191") in the property
 to properly run it.
*/


namespace gpudb
{
class GPUdb : private boost::noncopyable
{
public:

    class Options
    {
    public:
        Options();

#ifndef GPUDB_NO_HTTPS
        boost::asio::ssl::context* getSslContext() const;
        bool getBypassSslCertCheck() const;
#endif

        std::string getUsername() const;
        std::string getPassword() const;
        /// Return the URL of the primary cluster, if any (empty string
        /// delineates that none was set)
        std::string getPrimaryUrl() const;
        bool getUseSnappy() const;
        size_t getThreadCount() const;
        avro::ExecutorPtr getExecutor() const;

        /// Get the HTTP headers (will include the high-availability
        /// synchronicity override header)
        std::map<std::string, std::string>& getHttpHeaders();
        const std::map<std::string, std::string>& getHttpHeaders() const;

        size_t getTimeout() const;
        uint16_t getHostManagerPort() const;
        bool getDisableFailover() const;
        bool getDisableAutoDiscovery() const;

#ifndef GPUDB_NO_HTTPS
        Options& setSslContext(boost::asio::ssl::context* value);
        Options& setBypassSslCertCheck(const bool value);
#endif

        Options& setUsername(const std::string& value);
        Options& setPassword(const std::string& value);
        /// Set the URL for the primary cluster
        Options& setPrimaryUrl(const std::string& value);
        Options& setUseSnappy(const bool value);
        Options& setThreadCount(const size_t value);
        Options& setExecutor(const avro::ExecutorPtr value);
        Options& setHttpHeaders(const std::map<std::string, std::string>& value);
        Options& addHttpHeader(const std::string& header, const std::string& value);
        Options& setTimeout(const size_t value);
        Options& setHostManagerPort(const uint16_t value);
        Options& setDisableFailover(const bool value);
        Options& setDisableAutoDiscovery(const bool value);

    private:

#ifndef GPUDB_NO_HTTPS
        boost::asio::ssl::context* m_sslContext;
        bool m_bypassSslCertCheck;
#endif

        std::string m_username;
        std::string m_password;
        std::string m_primaryUrl;
        bool m_useSnappy;
        bool m_disableFailover;
        bool m_disableAutoDiscovery;
        size_t m_threadCount;
        avro::ExecutorPtr m_executor;
        std::map<std::string, std::string> m_httpHeaders;
        size_t   m_timeout;
        uint16_t m_hmPort;
    };


    /**
     * A enumeration of high-availability synchronicity override modes.
     *
     * Note: Internally, do not use the enum value as the mode to be set
     *       in the header; use the {@link
     *       #getHASynchronicityModeValue(HASynchronicityMode)} to get
     *       the approprivate stirng value instead.
     */
    enum HASynchronicityMode {
        // No override; defer to the HA process for synchronizing
        // endpoints (which has different logic for different endpoints)
        DEFAULT = 0,  // maps to 'default'
        // Explicitly do NOT replicate across the HA cluster
        NONE,         // maps to 'REPL_NONE'
        // Synchronize all endpoint calls
        SYNCHRONOUS,  // maps to 'REPL_SYNC'
        // Do NOT synchronize any endpoint call
        ASYNCHRONOUS  // maps to 'REPL_ASYNC'
    };


    
    static const int64_t END_OF_SET = -9999;

    /// Special error messages indicating that a connection failure happened
    /// (generally should trigger a high-availability failover if applicable)
    static const std::string DB_CONNECTION_RESET_ERROR_MESSAGE;
    static const std::string DB_CONNECTION_REFUSED_ERROR_MESSAGE;
    static const std::string DB_EXITING_ERROR_MESSAGE;
    static const std::string DB_OFFLINE_ERROR_MESSAGE;
    static const std::string DB_SYSTEM_LIMITED_ERROR_MESSAGE;
    static const std::string DB_HM_OFFLINE_ERROR_MESSAGE;
    
    /// Headers used internally; MUST add each of them to PROTECTED_HEADERS
    /// in the .cpp file
    static const std::string HEADER_AUTHORIZATION;
    static const std::string HEADER_CONTENT_TYPE;
    static const std::string HEADER_CONTENT_LENGTH;
    static const std::string HEADER_HA_SYNC_MODE;
    
    
    static inline std::string getApiVersion() { return GPUdb::API_VERSION; }

    /**
     * Pass a single HttpURL and options to instantiate a GPUdb object.
     *
     * @param[in] url  An HttpURL object containing the single host URL
     *                 for the client.  If no primary URL is specified via
     *                 the options, the given URL will be used as the primary
     *                 URL.
     * @param[in] options  An optional GPUdb::Options object containing
     *                     options, e.g. primary cluster URL, used to the
     *                     create the GPUdb object.
     *
     * @return the instantiated GPUdb object.
     */
    GPUdb(const HttpUrl& url, const Options& options = Options());

    /**
     * Pass a single or multiple, comma-separated URLs as a string
     * and optional options to instantiate a GPUdb object.
     *
     * @param[in] url  An std::string containing the one host URL
     *                 or a comma-separated string with multiple host
     *                 URLs for the client.  For example
     *                 'http://172.42.40.1:9191,,http://172.42.40.2:9191'.
     *                 If a single URL is given, and no primary URL is
     *                 specified via the options, the given URL will be used
     *                 as the primary URL.
     * @param[in] options  An optional GPUdb::Options object containing
     *                     options, e.g. primary cluster URL, used to the
     *                     create the GPUdb object.
     *
     * @return the instantiated GPUdb object.
     */
    GPUdb(const std::string& url, const Options& options = Options());

    /**
     * Pass multiple HttpURLs and optional options to instantiate a GPUdb
     * object.
     *
     * @param[in] urls  The host URLs for the client.  If a single URL is given,
     *                  and no primary URL is specified via the options, the
     *                  given URL will be used as the primary URL.
     * @param[in] options  An optional GPUdb::Options object containing
     *                     options, e.g. primary cluster URL, used to the
     *                     create the GPUdb object.
     *
     * @return the instantiated GPUdb object.
     */
    GPUdb(const std::vector<HttpUrl>& urls, const Options& options = Options());

    /**
     * Pass multiple strings, each containing a single URL, and optional options
     * to instantiate a GPUdb object.
     *
     * @param[in] urls  The host URLs for the client.  Each string must contain
     *                  a single valid URL.  If a single URL is given,
     *                  and no primary URL is specified via the options, the
     *                  given URL will be used as the primary URL.
     * @param[in] options  An optional GPUdb::Options object containing
     *                     options, e.g. primary cluster URL, used to the
     *                     create the GPUdb object.
     *
     * @return the instantiated GPUdb object.
     */
    GPUdb(const std::vector<std::string>& urls, const Options& options = Options());

    /// Destructor
    ~GPUdb();
    
    /// Some getters
    /// ------------
    const HttpUrl& getUrl() const;
    const std::vector<HttpUrl>& getUrls() const;
    const HttpUrl& getHmUrl() const;
    const std::vector<HttpUrl>& getHmUrls() const;

#ifndef GPUDB_NO_HTTPS
    boost::asio::ssl::context* getSslContext() const;
    bool getBypassSslCertCheck() const;
#endif

    const std::string& getUsername() const;
    const std::string& getPassword() const;

    /// Return a string containing the URL for the primary cluster; empty
    /// string otherwise
    const std::string& getPrimaryURL() const;

    bool getUseSnappy() const;
    size_t getThreadCount() const;
    avro::ExecutorPtr getExecutor() const;
    const std::map<std::string, std::string>& getHttpHeaders() const;
    HASynchronicityMode getHASyncMode() const;
    size_t getTimeout() const;

    /**
     * This method is used to execute a SQL statement (e.g., DML, DDL).  It returns the number of
     * rows affected by the statement.
     * 
     * @param sql        - The SQL query to execute
     * @param parameters - Query parameters for the SQL query.  Can be empty.
     * @param options    - Optional parameters for the executeSql call.
     * 
     * @return - number of rows affected by the execution of statement
     */
    long execute(const std::string& sql);
    long execute(const std::string& sql, const std::string& parameters);
    long execute(const std::string& sql, const std::string& parameters,
                 const std::map<std::string, std::string>& options);
    template<typename T> long execute(const std::string& sql, const std::vector<T>& parameters);
    template<typename T> long execute(const std::string& sql, const std::vector<T>& parameters,
                                      const std::map<std::string, std::string>& options);

    /// Some setters
    /// ------------

    /**
     * Sets the high-availability synchronization mode which will override
     * the default mode.
     */
    void setHASyncMode( HASynchronicityMode mode );

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
    void addHttpHeader( const std::string& header,
                        const std::string& value );

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
    void removeHttpHeader( const std::string& header );

    
    /// Update the host manager port by inquiring the server
    void updateHostManagerPort();
    
    template<typename TRequest, typename TResponse>
    TResponse& submitRequest(const HttpUrl& url,
                             const TRequest& request,
                             TResponse& response,
                             const bool enableCompression = false) const
    {
        std::vector<uint8_t> requestBytes;
        try // encoding the request
        {
            avro::encode(requestBytes, request);
        }
        catch ( const std::exception& ex )
        {
            std::string message = GPUDB_STREAM_TO_STRING( "Error encountered while encoding request: "
                                                          << ex.what() );
            throw GPUdbException( message );
        }

        // Submit the request
        RawGpudbResponse gpudbResponse;
        submitRequestRaw(url, requestBytes, gpudbResponse, enableCompression);

        try // decoding the request
        {
            avro::decode(response, gpudbResponse.data);
        }
        catch ( const std::exception& ex )
        {
            std::string message = GPUDB_STREAM_TO_STRING( "Error encountered while decoding response: '"
                                                          << ex.what()
                                                          << "'; please ensure that the client API matches the server API version." );
            throw GPUdbException( message );
        }
        return response;
    }

    template<typename TRequest, typename TResponse>
    TResponse& submitRequest(const std::string& endpoint,
                             const TRequest& request,
                             TResponse& response,
                             const bool enableCompression = false) const
    {
        std::vector<uint8_t> requestBytes;
        try // encoding the request
        {
            avro::encode(requestBytes, request);
        }
        catch ( const std::exception& ex )
        {
            std::string message = GPUDB_STREAM_TO_STRING( "Error encountered while encoding request: "
                                                          << ex.what() );
            throw GPUdbException( message );
        }

        // Submit the request
        RawGpudbResponse gpudbResponse;
        submitRequestRaw(endpoint, requestBytes, gpudbResponse, enableCompression);

        try // decoding the request
        {
            avro::decode(response, gpudbResponse.data);
        }
        catch ( const std::exception& ex )
        {
            std::string message = GPUDB_STREAM_TO_STRING( "Error encountered while decoding response: '"
                                                          << ex.what()
                                                          << "'; please ensure that the client API matches the server API version." );
            throw GPUdbException( message );
        }

        return response;
    }

    template<typename TRequest, typename TResponse>
    TResponse& submitRequest(const char* endpoint,
                             const TRequest& request,
                             TResponse& response,
                             const bool enableCompression = false) const
    {
        submitRequest( (std::string) endpoint, request, response, enableCompression );
        return response;
    }


    /**
     * Submit an HTTP request to the host manager.
     * @param[in]  endpoint  The endpoint for which the request is made.
     * @param[in]  request   The request object containing the parameters for
     *                       the operation.
     * @param[out] response  The response object in which the return values will
     *                       be saved.
     * @param[in]  enableCompression  Optional boolean flag indicating whether
     *                                any compression should be used.  Default is
     *                                false.
     *
     * @return a reference to the response object.
     */
    template<typename TRequest, typename TResponse>
    TResponse& submitRequestToHostManager(const std::string& endpoint,
                                          const TRequest& request,
                                          TResponse& response,
                                          const bool enableCompression = false) const
    {
        // Handle host manager stuff here
        std::vector<uint8_t> requestBytes;
        try // encoding the request
        {
        avro::encode(requestBytes, request);
        }
        catch ( const std::exception& ex )
        {
            std::string message = GPUDB_STREAM_TO_STRING( "Error encountered while encoding request: "
                                                          << ex.what() );
            throw GPUdbException( message );
        }

        // Submit the request
        RawGpudbResponse gpudbResponse;
        submitRequestToHostManagerRaw(endpoint, requestBytes, gpudbResponse, enableCompression);

        try // decoding the request
        {
            avro::decode(response, gpudbResponse.data);
        }
        catch ( const std::exception& ex )
        {
            std::string message = GPUDB_STREAM_TO_STRING( "Error encountered while decoding response: '"
                                                          << ex.what()
                                                          << "'; please ensure that the client API matches the server API version." );
            throw GPUdbException( message );
        }
        return response;
    }   // end submitRequestToHostManager


    /**
     * Submit an HTTP request to the host manager.
     * @param[in]  endpoint  The endpoint for which the request is made.
     * @param[in]  request   The request object containing the parameters for
     *                       the operation.
     * @param[out] response  The response object in which the return values will
     *                       be saved.
     * @param[in]  enableCompression  Optional boolean flag indicating whether
     *                                any compression should be used.  Default is
     *                                false.
     *
     * @return a reference to the response object.
     */
    template<typename TRequest, typename TResponse>
    TResponse& submitRequestToHostManager(const char* endpoint,
                                          const TRequest& request,
                                          TResponse& response,
                                          const bool enableCompression = false) const
    {
        submitRequestToHostManager( (std::string) endpoint, request, response, enableCompression );
        return response;
    }   // end submitRequestToHostManager


#include "gpudb/GPUdbFunctions.hpp"

    void addKnownType(const std::string& typeId, const avro::DecoderPtr& decoder);

    template<typename T>
    void addKnownType(const std::string& typeId)
    {
        addKnownType(typeId, avro::createDecoder<T>());
    }

    template<typename T>
    void addKnownType(const std::string& typeId, const std::string& schemaString)
    {
        addKnownType(typeId, avro::createDecoder<T>(schemaString));
    }

    template<typename T>
    void addKnownType(const std::string& typeId, const ::avro::ValidSchema& schema)
    {
        addKnownType(typeId, avro::createDecoder<T>(schema));
    }

    void addKnownTypeFromTable(const std::string& tableName, const avro::DecoderPtr& decoder);

    template<typename T>
    void addKnownTypeFromTable(const std::string& tableName)
    {
        addKnownTypeFromTable(tableName, avro::createDecoder<T>());
    }

    template<typename T>
    void addKnownTypeFromTable(const std::string& tableName, const std::string& schemaString)
    {
        addKnownTypeFromTable(tableName, avro::createDecoder<T>(schemaString));
    }

    template<typename T>
    void addKnownTypeFromTable(const std::string& tableName, const ::avro::ValidSchema& schema)
    {
        addKnownTypeFromTable(tableName, avro::createDecoder<T>(schema));
    }

private:

    static const std::string API_VERSION;

    static const std::string FAILOVER_TRIGGER_MESSAGES[];
    static const std::string PROTECTED_HEADERS[];
    static const std::string HA_SYNCHRONICITY_MODE_VALUES[];

    static const size_t NUM_TRIGGER_MESSAGES;
    
    mutable std::vector<HttpUrl> m_urls;
    mutable std::vector<HttpUrl> m_hmUrls;
    std::string m_primaryUrlStr;
    HttpUrl* m_primaryUrlPtr;
    mutable std::vector<size_t>  m_urlIndices;
    mutable boost::mutex m_urlMutex;
    mutable size_t m_currentUrl;

#ifndef GPUDB_NO_HTTPS
    boost::asio::ssl::context* m_sslContext;
#endif

    std::string m_username;
    std::string m_password;
    std::string m_authorization;
    bool m_useSnappy;
    bool m_disableFailover;
    bool m_bypassSslCertCheck;
    bool m_disableAutoDiscovery;
    size_t m_threadCount;
    avro::ExecutorPtr m_executor;
    std::map<std::string, std::string> m_httpHeaders;
    size_t m_timeout;
    mutable HASynchronicityMode m_haSyncMode;
    Options m_options;

    mutable std::map<std::string, avro::DecoderPtr> m_knownTypes;
    mutable boost::mutex m_knownTypesMutex;

   
    /// Helper functions
    /// ----------------
    /// Initialize the GPUdb object
    void init();

    // Handle the primary host URL, if any is given via options
    void handlePrimaryURL();

    // Update the URLs with the available HA ring information
    void getHAringHeadNodeAddresses();

    /// Host manager related methods
    void updateHostManagerUrls();
    void setHostManagerPort(uint16_t value);

    /// Randomly shuffles the list of high availability URL indices so that HA
    /// failover happens at a random fashion.  One caveat is when a primary host
    /// is given by the user; in that case, we need to keep the primary host's
    /// index as the first one in the list so that upon failover, when we cricle
    /// back, we always pick the first/primary host up again.
    void randomizeURLs() const;

    /// Convert a given high availability synchronicity override mode enumeration
    /// to its string value
    const std::string& getHASynchronicityModeValue( HASynchronicityMode syncMode ) const;

    /// Some getters
    /// ------------
    const HttpUrl* getUrlPointer() const;
    const HttpUrl* getHmUrlPointer() const;
    const HttpUrl* switchUrl(const HttpUrl* oldUrl) const;
    const HttpUrl* switchHmUrl(const HttpUrl* oldUrl) const;

    
    /// Request related methods
    /// -----------------------
    void initHttpRequest(HttpRequest& httpRequest) const;
    void submitRequestRaw(const std::string& endpoint,
                          const std::vector<uint8_t>& request,
                          RawGpudbResponse& response,
                          const bool enableCompression) const;
    void submitRequestToHostManagerRaw(const std::string& endpoint,
                                       const std::vector<uint8_t>& request,
                                       RawGpudbResponse& response,
                                       const bool enableCompression) const;
    void submitRequestRaw(const HttpUrl& url,
                          const std::vector<uint8_t>& request,
                          RawGpudbResponse& response,
                          const bool enableCompression,
                          const bool throwOnError = true) const;

    /// Encoding related methods
    avro::DecoderPtr getDecoder(const std::string& typeId) const;
    void setDecoderIfMissing(const std::string& typeId,
                             const std::string& label,
                             const std::string& schemaString,
                             const std::map<std::string, std::vector<std::string> >& properties) const;

};

#include "gpudb/GPUdbTemplates.hpp"
    
}  // end namespace gpudb

#endif
