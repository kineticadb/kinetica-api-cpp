#include "gpudb/GPUdb.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <snappy.h>

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
        m_currentUrl(0),

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
        m_timeout(options.getTimeout())
    {
        m_urls.push_back(url);
    }

    GPUdb::GPUdb(const std::string& url, const Options& options) :
        m_currentUrl(0),

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
        m_timeout(options.getTimeout())
    {
        m_urls.push_back(HttpUrl(url));
    }

    GPUdb::GPUdb(const std::vector<HttpUrl>& urls, const Options& options) :
        m_urls(urls),
        m_currentUrl(randomItem(urls.size())),

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
        m_timeout(options.getTimeout())
    {
        if (urls.empty())
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }
    }

    GPUdb::GPUdb(const std::vector<std::string>& urls, const Options& options) :
        m_currentUrl(randomItem(urls.size())),

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
        m_timeout(options.getTimeout())
    {
        if (urls.empty())
        {
            throw std::invalid_argument("At least one URL must be specified.");
        }

        m_urls.reserve(urls.size());
        m_urls.insert(m_urls.end(), urls.begin(), urls.end());
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

            return m_urls[m_currentUrl];
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

            return &m_urls[m_currentUrl];
        }
    }

    const std::vector<HttpUrl>& GPUdb::getUrls() const
    {
        return m_urls;
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
                                 GpudbResponse& response,
                                 const bool enableCompression) const
    {
        const HttpUrl* url = getUrlPointer();
        const HttpUrl* originalUrl = url;

        while (true)
        {
            try
            {
                submitRequestRaw(HttpUrl(*url, endpoint), request, response, enableCompression, false);
                break;
            }
            catch (const std::exception& /*ex*/)
            {
                if (m_urls.size() == 1)
                {
                    throw;
                }
                else
                {
                    url = switchUrl(url);

                    if (url == originalUrl)
                    {
                        throw;
                    }
                }
            }
        }

        if (response.status == "ERROR")
        {
            throw GPUdbException(response.message);
        }
    }

    void GPUdb::submitRequestRaw(const HttpUrl& url,
                                 const std::vector<uint8_t>& request,
                                 GpudbResponse& response,
                                 const bool enableCompression,
                                 const bool throwOnError) const
    {
        BinaryHttpResponse httpResponse;

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

        if (throwOnError && response.status == "ERROR")
        {
            throw GPUdbException(response.message);
        }
    }

    const HttpUrl* GPUdb::switchUrl(const HttpUrl* oldUrl) const
    {
        boost::mutex::scoped_lock lock(m_urlMutex);

        if (&m_urls[m_currentUrl] == oldUrl)
        {
            m_currentUrl++;

            if (m_currentUrl >= m_urls.size())
            {
                m_currentUrl = 0;
            }
        }

        return &m_urls[m_currentUrl];
    }

    GPUdb::Options::Options() :
        #ifndef GPUDB_NO_HTTPS
        m_sslContext(NULL),
        #endif

        m_useSnappy(true),
        m_threadCount(1),
        m_timeout(0)
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
