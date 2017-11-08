#ifndef __GPUDB_HPP__
#define __GPUDB_HPP__

#include "gpudb/protocol/EndpointKeywords.h"

#ifndef GPUDB_NO_HTTPS
#include <boost/asio/ssl.hpp>
#endif

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
                    size_t m_timeout;
            };

            static const int64_t END_OF_SET = -9999;

            static inline std::string getApiVersion() { return GPUdb::API_VERSION; }

            GPUdb(const HttpUrl& url, const Options& options = Options());
            GPUdb(const std::string& url, const Options& options = Options());
            GPUdb(const std::vector<HttpUrl>& urls, const Options& options = Options());
            GPUdb(const std::vector<std::string>& urls, const Options& options = Options());
            const HttpUrl& getUrl() const;
            const std::vector<HttpUrl>& getUrls() const;

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

            template<typename TRequest, typename TResponse>
            TResponse& submitRequest(const HttpUrl& url,
                                     const TRequest& request,
                                     TResponse& response,
                                     const bool enableCompression = false) const
            {
                std::vector<uint8_t> requestBytes;
                avro::encode(requestBytes, request);
                GpudbResponse gpudbResponse;
                submitRequestRaw(url, requestBytes, gpudbResponse, enableCompression);
                avro::decode(response, gpudbResponse.data);
                return response;
            }

            template<typename TRequest, typename TResponse>
            TResponse& submitRequest(const std::string& endpoint,
                                     const TRequest& request,
                                     TResponse& response,
                                     const bool enableCompression = false) const
            {
                std::vector<uint8_t> requestBytes;
                avro::encode(requestBytes, request);
                GpudbResponse gpudbResponse;
                submitRequestRaw(endpoint, requestBytes, gpudbResponse, enableCompression);
                avro::decode(response, gpudbResponse.data);
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

            mutable std::map<std::string, avro::DecoderPtr> m_knownTypes;
            mutable boost::mutex m_knownTypesMutex;

            const HttpUrl* getUrlPointer() const;
            const HttpUrl* switchUrl(const HttpUrl* oldUrl) const;
            void initHttpRequest(HttpRequest& httpRequest) const;
            void submitRequestRaw(const std::string& endpoint,
                                  const std::vector<uint8_t>& request,
                                  GpudbResponse& response,
                                  const bool enableCompression) const;
            void submitRequestRaw(const HttpUrl& url,
                                  const std::vector<uint8_t>& request,
                                  GpudbResponse& response,
                                  const bool enableCompression,
                                  const bool throwOnError = true) const;
            avro::DecoderPtr getDecoder(const std::string& typeId) const;
            void setDecoderIfMissing(const std::string& typeId,
                                     const std::string& label,
                                     const std::string& schemaString,
                                     const std::map<std::string, std::vector<std::string> >& properties) const;
    };

    #include "gpudb/GPUdbTemplates.hpp"
}

#endif
