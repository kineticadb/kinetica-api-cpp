#ifndef __GPUDB_HPP__
#define __GPUDB_HPP__

#include <boost/thread/mutex.hpp>

namespace gpudb
{
    class GPUdb;
}

#include "Avro.hpp"
#include "GenericRecord.hpp"
#include "GPUdbException.hpp"
#include "Type.hpp"
#include "DynamicTableRecord.hpp"
#include "protocol/GPUdbProtocol.h"

namespace gpudb
{
    class GPUdb : private boost::noncopyable
    {
        public:
            class Options
            {
                public:
                    Options();
                    std::string getUsername() const;
                    std::string getPassword() const;
                    std::string getUserAuth() const;
                    bool getUseSnappy() const;
                    size_t getThreadCount() const;
                    avro::ExecutorPtr getExecutor() const;
                    Options& setUsername(const std::string& value);
                    Options& setPassword(const std::string& value);
                    Options& setUserAuth(const std::string& value);
                    Options& setUseSnappy(const bool value);
                    Options& setThreadCount(const size_t value);
                    Options& setExecutor(const avro::ExecutorPtr value);

                private:
                    std::string username;
                    std::string password;
                    std::string userAuth;
                    bool useSnappy;
                    size_t threadCount;
                    avro::ExecutorPtr executor;
            };

            GPUdb(const std::string& url, const Options& options = Options());
            std::string getUrl() const;
            std::string getUsername() const;
            std::string getPassword() const;
            std::string getUserAuth() const;
            bool getUseSnappy() const;
            size_t getThreadCount() const;
            avro::ExecutorPtr getExecutor() const;

            template<typename TRequest, typename TResponse> TResponse& submitRequest(const std::string& endpoint, const TRequest& request, TResponse& response, const bool enableCompression = false) const
            {
                std::vector<uint8_t> requestBytes;
                avro::encode(requestBytes, request);
                GaiaResponse gaiaResponse;
                submitRequestRaw(endpoint, requestBytes, gaiaResponse, enableCompression);
                avro::decode(response, gaiaResponse.data);
                return response;
            }

            #include "GPUdbFunctions.hpp"

            void addKnownType(const std::string& typeId, const avro::DecoderPtr& decoder);

            template<typename T> void addKnownType(const std::string& typeId)
            {
                addKnownType(typeId, avro::createDecoder<T>());
            }

            template<typename T> void addKnownType(const std::string& typeId, const std::string& schemaString)
            {
                addKnownType(typeId, avro::createDecoder<T>(schemaString));
            }

            template<typename T> void addKnownType(const std::string& typeId, const ::avro::ValidSchema& schema)
            {
                addKnownType(typeId, avro::createDecoder<T>(schema));
            }

            void addKnownTypeFromTable(const std::string& tableName, const avro::DecoderPtr& decoder);

            template<typename T> void addKnownTypeFromTable(const std::string& tableName)
            {
                addKnownTypeFromTable(tableName, avro::createDecoder<T>());
            }

            template<typename T> void addKnownTypeFromTable(const std::string& tableName, const std::string& schemaString)
            {
                addKnownTypeFromTable(tableName, avro::createDecoder<T>(schemaString));
            }

            template<typename T> void addKnownTypeFromTable(const std::string& tableName, const ::avro::ValidSchema& schema)
            {
                addKnownTypeFromTable(tableName, avro::createDecoder<T>(schema));
            }

        private:
            std::string url;
            std::string host;
            std::string service;
            std::string path;
            bool secure;
            std::string username;
            std::string password;
            std::string authorization;
            std::string userAuth;
            bool useSnappy;
            size_t threadCount;
            avro::ExecutorPtr executor;
            mutable std::map<std::string, avro::DecoderPtr> knownTypes;
            mutable boost::mutex knownTypesMutex;

            void submitRequestRaw(const std::string& endpoint, const std::vector<uint8_t>& request, GaiaResponse& response, const bool enableCompression) const;
            avro::DecoderPtr getDecoder(const std::string& typeId) const;
            void setDecoderIfMissing(const std::string& typeId, const std::string& schemaString) const;
    };

    #include "GPUdbTemplates.hpp"
}

#endif
