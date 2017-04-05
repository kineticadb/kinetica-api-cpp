#ifndef __GPUDB_HPP__
#define __GPUDB_HPP__

#include "gpudb/protocol/EndpointKeywords.h"

#include <boost/thread/mutex.hpp>

namespace gpudb
{
    class GPUdb;
}

#include "gpudb/Avro.hpp"
#include "gpudb/GenericRecord.hpp"
#include "gpudb/GPUdbException.hpp"
#include "gpudb/Type.hpp"
#include "gpudb/protocol/GPUdbProtocol.h"

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
                    bool getUseSnappy() const;
                    size_t getThreadCount() const;
                    avro::ExecutorPtr getExecutor() const;
                    Options& setUsername(const std::string& value);
                    Options& setPassword(const std::string& value);
                    Options& setUseSnappy(const bool value);
                    Options& setThreadCount(const size_t value);
                    Options& setExecutor(const avro::ExecutorPtr value);

                private:
                    std::string username;
                    std::string password;
                    bool useSnappy;
                    size_t threadCount;
                    avro::ExecutorPtr executor;
            };

            static const int64_t END_OF_SET = -9999;

            static inline std::string getApiVersion() { return GPUdb::API_VERSION; }

            GPUdb(const std::string& url, const Options& options = Options());
            GPUdb(const std::vector<std::string>& urls, const Options& options = Options());
            std::string getUrl() const;
            std::string getUsername() const;
            std::string getPassword() const;
            bool getUseSnappy() const;
            size_t getThreadCount() const;
            avro::ExecutorPtr getExecutor() const;

            template<typename TRequest, typename TResponse> TResponse& submitRequest(const std::string& endpoint, const TRequest& request, TResponse& response, const bool enableCompression = false) const
            {
                std::vector<uint8_t> requestBytes;
                avro::encode(requestBytes, request);
                GpudbResponse gpudbResponse;
                submitRequestRaw(endpoint, requestBytes, gpudbResponse, enableCompression);
                avro::decode(response, gpudbResponse.data);
                return response;
            }

            #include "gpudb/GPUdbFunctions.hpp"

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
            static const std::string API_VERSION;

            /*
             * Wraps connection params together for cycling in the HA logic
             */
            struct ConnectionToken
            {
                ConnectionToken(const std::string& url);
                bool operator == (const ConnectionToken &rhs) const;
                bool operator != (const ConnectionToken &rhs) const;

                std::string url;
                std::string host;
                std::string port;
                std::string path;
                bool secure;
            };

            /*
             * Manages the list of available hosts for HA. Randomly selects
             */
            struct ConnectionTokenManager
            {
                /*
                 * Populates the list and randomly assigns the currentIndex
                 */
                void initialize(const std::vector<ConnectionToken>& tokens);

                /*
                 * @return The currently available connection information
                 */
                ConnectionToken getCurrentToken() const;

                /*
                 * Cycles the index to the next entry (wrapping around if
                 * necessary)
                 * @return The new current token
                 */
                ConnectionToken getNextToken();

                private:
                    std::size_t currentIndex;
                    std::vector<ConnectionToken> tokens;
                    mutable boost::mutex tokensMutex;
            };

            void initializeConnectionTokens(
                const std::vector<std::string>& urls);

            // Mutable for usage in submitRequestRaw
            mutable ConnectionTokenManager tokenManager;

            std::string username;
            std::string password;
            std::string authorization;
            bool useSnappy;
            size_t threadCount;
            avro::ExecutorPtr executor;
            mutable std::map<std::string, avro::DecoderPtr> knownTypes;
            mutable boost::mutex knownTypesMutex;

            void submitRequestRaw(const std::string& endpoint, const std::vector<uint8_t>& request, GpudbResponse& response, const bool enableCompression) const;
            avro::DecoderPtr getDecoder(const std::string& typeId) const;
            void setDecoderIfMissing(const std::string& typeId, const std::string& label, const std::string& schemaString, const std::map<std::string, std::vector<std::string> >& properties) const;
    };

    #include "gpudb/GPUdbTemplates.hpp"
}

#endif
