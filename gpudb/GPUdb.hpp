#ifndef __GPUDB_HPP__
#define __GPUDB_HPP__

#include "gpudb/protocol/EndpointKeywords.h"
#include "gpudb/utils/Utils.h"

#ifndef GPUDB_NO_HTTPS
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

 The source code can be found <a href="https://github.com/kineticadb/kinetica-api-cpp">here</a>.

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
#endif

        std::string getUsername() const;
        std::string getPassword() const;
        bool getUseSnappy() const;
        size_t getThreadCount() const;
        avro::ExecutorPtr getExecutor() const;
        std::map<std::string, std::string>& getHttpHeaders();
        const std::map<std::string, std::string>& getHttpHeaders() const;
        size_t getTimeout() const;
        uint16_t getHostManagerPort() const;

#ifndef GPUDB_NO_HTTPS
        Options& setSslContext(boost::asio::ssl::context* value);
#endif

        Options& setUsername(const std::string& value);
        Options& setPassword(const std::string& value);
        Options& setUseSnappy(const bool value);
        Options& setThreadCount(const size_t value);
        Options& setExecutor(const avro::ExecutorPtr value);
        Options& setHttpHeaders(const std::map<std::string, std::string>& value);
        Options& addHttpHeader(const std::string& header, const std::string& value);
        Options& setTimeout(const size_t value);
        Options& setHostManagerPort(const uint16_t value);

    private:

#ifndef GPUDB_NO_HTTPS
        boost::asio::ssl::context* m_sslContext;
#endif

        std::string m_username;
        std::string m_password;
        bool m_useSnappy;
        size_t m_threadCount;
        avro::ExecutorPtr m_executor;
        std::map<std::string, std::string> m_httpHeaders;
        size_t   m_timeout;
        uint16_t m_hmPort;
    };

    static const int64_t END_OF_SET = -9999;

    static inline std::string getApiVersion() { return GPUdb::API_VERSION; }

    /// Pass a single HttpURL and options to instantiate a GPUdb object
    GPUdb(const HttpUrl& url, const Options& options = Options());

    /// Pass a single or multiple, comma-separated URLs as a string and options
    /// to instantiate a GPUdb object
    GPUdb(const std::string& url, const Options& options = Options());

    /// Pass multiple HttpURLs and options to instatiate a GPUdb object
    GPUdb(const std::vector<HttpUrl>& urls, const Options& options = Options());

    /// Pass multiple strings, each containing a single URL, and options to
    /// instantiate a GPUdb object
    GPUdb(const std::vector<std::string>& urls, const Options& options = Options());

    /// Some getters
    const HttpUrl& getUrl() const;
    const std::vector<HttpUrl>& getUrls() const;
    const HttpUrl& getHmUrl() const;
    const std::vector<HttpUrl>& getHmUrls() const;

#ifndef GPUDB_NO_HTTPS
    boost::asio::ssl::context* getSslContext() const;
#endif

    const std::string& getUsername() const;
    const std::string& getPassword() const;
    bool getUseSnappy() const;
    size_t getThreadCount() const;
    avro::ExecutorPtr getExecutor() const;
    const std::map<std::string, std::string>& getHttpHeaders() const;
    size_t getTimeout() const;

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

    std::vector<HttpUrl> m_urls;
    std::vector<HttpUrl> m_hmUrls;
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
    size_t m_threadCount;
    avro::ExecutorPtr m_executor;
    std::map<std::string, std::string> m_httpHeaders;
    size_t m_timeout;
    Options m_options;

    mutable std::map<std::string, avro::DecoderPtr> m_knownTypes;
    mutable boost::mutex m_knownTypesMutex;

    /// Helper functions
    void init();
    
    /// Some getters
    const HttpUrl* getUrlPointer() const;
    const HttpUrl* getHmUrlPointer() const;
    const HttpUrl* switchUrl(const HttpUrl* oldUrl) const;
    const HttpUrl* switchHmUrl(const HttpUrl* oldUrl) const;

    /// Request related methods
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

    /// Host manager related methods
    void createHostManagerUrl( const HttpUrl& url, uint16_t hostManagerPort );
    void createHostManagerUrls( const std::vector<HttpUrl>& urls, uint16_t hostManagerPort );
    void setHostManagerPort(uint16_t value);
};

#include "gpudb/GPUdbTemplates.hpp"
    
}  // end namespace gpudb

#endif
