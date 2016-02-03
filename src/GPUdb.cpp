#include "GPUdb.hpp"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
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

    bool parseUrl(const std::string& url, std::string& host, std::string& port, std::string& path, bool& secure)
    {
        size_t i = 0;

        std::string protocol;
        protocol.reserve(8);

        for (; i < url.length(); ++i)
        {
            protocol.push_back(url[i]);

            if (i > 1 && url.substr(i - 2, 3) == "://")
            {
                break;
            }
        }

        if (protocol == "http://")
        {
            secure = false;
        }
        else if (protocol == "https://")
        {
            secure = true;
        }
        else
        {
            return false;
        }

        host.clear();
        host.reserve(url.length());
        bool portSpecified = false;

        for (++i; i < url.length(); ++i)
        {
            if (url[i] == ':')
            {
                portSpecified = true;
                break;
            }
            else if (url[i] == '/')
            {
                portSpecified = false;
                break;
            }

            host.push_back(url[i]);
        }

        if (host.empty())
        {
            return false;
        }

        if (portSpecified)
        {
            port.clear();
            port.reserve(5);

            for (++i; i < url.length(); ++i)
            {
                if (url[i] == '/')
                {
                    break;
                }
                else if (url[i] < '0' || url[i] > '9')
                {
                    return false;
                }

                port.push_back(url[i]);
            }

            try
            {
                boost::lexical_cast<uint16_t>(port);
            }
            catch (boost::bad_lexical_cast&)
            {
                return false;
            }
        }
        else if (secure)
        {
            port = "443";
        }
        else
        {
            port = "80";
        }

        path.clear();
        path.reserve(url.length());

        for (; i < url.length(); ++i)
        {
            if (url[i] == '?')
            {
                return false;
            }

            path.push_back(url[i]);
        }

        if (path[path.length() - 1] == '/')
        {
            path.resize(path.length() - 1);
        }

        return true;
    }

    GPUdb::GPUdb(const std::string& url, const Options& options) :
        url(url),
        username(options.getUsername()),
        password(options.getPassword()),
        authorization((!options.getUsername().empty() || !options.getPassword().empty()) ? base64Encode(options.getUsername() + ":" + options.getPassword()) : ""),
        userAuth(options.getUserAuth()),
        useSnappy(options.getUseSnappy()),
        threadCount(options.getThreadCount()),
        executor(options.getExecutor())
    {
        if (!parseUrl(url, host, service, path, secure))
        {
            throw std::invalid_argument("Invalid URL specified.");
        }

        if (secure)
        {
            throw std::invalid_argument("HTTPS not supported.");
        }
    }

    void GPUdb::addKnownType(const std::string& typeId, const avro::DecoderPtr& decoder)
    {
        if (!decoder)
        {
            boost::mutex::scoped_lock lock(knownTypesMutex);
            knownTypes.erase(typeId);
        }
        else
        {
            boost::mutex::scoped_lock lock(knownTypesMutex);
            knownTypes[typeId] = decoder;
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

        for (std::vector<std::string>::const_iterator it = response.typeIds.begin(); it != response.typeIds.end(); ++it)
        {
            if (*it != typeId)
            {
                throw GPUdbException("Table " + tableName + " is not homogeneous.");
            }
        }

        if (!decoder)
        {
            boost::mutex::scoped_lock lock(knownTypesMutex);
            knownTypes.erase(typeId);
        }
        else
        {
            boost::mutex::scoped_lock lock(knownTypesMutex);
            knownTypes[typeId] = decoder;
        }
    }

    avro::DecoderPtr GPUdb::getDecoder(const std::string& typeId) const
    {
        {
            boost::mutex::scoped_lock lock(knownTypesMutex);

            if (knownTypes.find(typeId) != knownTypes.end())
            {
                return knownTypes[typeId];
            }
        }

        ShowTypesRequest request(typeId, "", std::map<std::string, std::string>());
        ShowTypesResponse response;
        submitRequest("/show/types", request, response);

        if (response.typeSchemas.size() == 0)
        {
            throw GPUdbException("Unable to obtain type information for type " + typeId + ".");
        }

        ::avro::ValidSchema schema = ::avro::compileJsonSchemaFromString(response.typeSchemas[0]);
        avro::DecoderPtr decoder = avro::createDecoder<GenericRecord>(schema);
        boost::mutex::scoped_lock lock(knownTypesMutex);

        if (knownTypes.find(typeId) != knownTypes.end())
        {
            return knownTypes[typeId];
        }

        knownTypes[typeId] = decoder;
        return decoder;
    }

    avro::ExecutorPtr GPUdb::getExecutor() const
    {
        return executor;
    }

    std::string GPUdb::getPassword() const
    {
        return password;
    }

    size_t GPUdb::getThreadCount() const
    {
        return threadCount;
    }

    std::string GPUdb::getUrl() const
    {
        return url;
    }

    bool GPUdb::getUseSnappy() const
    {
        return useSnappy;
    }

    std::string GPUdb::getUserAuth() const
    {
        return userAuth;
    }

    std::string GPUdb::getUsername() const
    {
        return username;
    }

    void GPUdb::setDecoderIfMissing(const std::string& typeId, const std::string& schemaString) const
    {
        {
            boost::mutex::scoped_lock lock(knownTypesMutex);

            if (knownTypes.find(typeId) != knownTypes.end())
            {
                return;
            }
        }

        ::avro::ValidSchema schema = ::avro::compileJsonSchemaFromString(schemaString);
        avro::DecoderPtr decoder = avro::createDecoder<GenericRecord>(schema);
        boost::mutex::scoped_lock lock(knownTypesMutex);

        if (knownTypes.find(typeId) != knownTypes.end())
        {
            return;
        }

        knownTypes[typeId] = decoder;
    }

    void GPUdb::submitRequestRaw(const std::string& endpoint, const std::vector<uint8_t>& request, GaiaResponse& response, const bool enableCompression) const
    {
        try
        {
            boost::asio::io_service ioService;
            boost::asio::ip::tcp::resolver resolver(ioService);
            boost::asio::ip::tcp::resolver::query query(host, service);
            boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
            boost::asio::ip::tcp::socket socket(ioService);
            boost::asio::connect(socket, iterator);
            boost::asio::streambuf requestStreamBuffer;
            std::ostream requestStream(&requestStreamBuffer);
            requestStream << "POST " << path << endpoint << " HTTP/1.0\r\n";

            if (!userAuth.empty())
            {
                requestStream << "GPUdb-User-Auth: " << userAuth << "\r\n";
            }

            if (!authorization.empty())
            {
                requestStream << "Authorization: Basic " << authorization << "\r\n";
            }

            if (enableCompression && useSnappy)
            {
                std::string compressedRequest;
                snappy::Compress((char*)&request[0], request.size(), &compressedRequest);
                requestStream << "Content-Type: application/x-snappy\r\n";
                requestStream << "Content-Length: " << compressedRequest.size() << "\r\n";
                requestStream << "\r\n";
                boost::asio::write(socket, requestStreamBuffer);
                boost::asio::write(socket, boost::asio::buffer(compressedRequest));
            }
            else
            {
                requestStream << "Content-Type: application/octet-stream\r\n";
                requestStream << "Content-Length: " << request.size() << "\r\n";
                requestStream << "\r\n";
                boost::asio::write(socket, requestStreamBuffer);
                boost::asio::write(socket, boost::asio::buffer(request));
            }

            boost::asio::streambuf responseStreamBuffer;
            boost::asio::read_until(socket, responseStreamBuffer, "\r\n\r\n");
            std::istream responseStream(&responseStreamBuffer);
            std::string header;
            std::getline(responseStream, header);

            if (!responseStream || header.substr(0, 5) != "HTTP/")
            {
                throw GPUdbException("Invalid response from server.");
            }

            do
            {
                std::getline(responseStream, header);
            }
            while (header != "\r");

            std::vector<uint8_t> responseBytes;
            boost::system::error_code error;

            do
            {
                boost::asio::streambuf::const_buffers_type data = responseStreamBuffer.data();
                size_t totalRead = 0;

                for (boost::asio::streambuf::const_buffers_type::const_iterator it = data.begin(); it != data.end(); ++it)
                {
                    const uint8_t* buffer = boost::asio::buffer_cast<const uint8_t*>(*it);
                    size_t read = boost::asio::buffer_size(*it);
                    responseBytes.insert(responseBytes.end(), buffer, buffer + read);
                    totalRead += read;
                }

                responseStreamBuffer.consume(totalRead);
            }
            while (boost::asio::read(socket, responseStreamBuffer, boost::asio::transfer_at_least(1), error));

            if (error != boost::asio::error::eof)
            {
                throw boost::system::system_error(error);
            }

            avro::decode(response, responseBytes);

            if (response.status == "ERROR")
            {
                throw GPUdbException(response.message);
            }
        }
        catch (const boost::system::system_error& ex)
        {
            throw GPUdbException(std::string(ex.what()));
        }
    }

    GPUdb::Options::Options() :
        useSnappy(false),
        threadCount(1)
    {
    }

    avro::ExecutorPtr GPUdb::Options::getExecutor() const
    {
        return executor;
    }

    std::string GPUdb::Options::getPassword() const
    {
        return password;
    }

    size_t GPUdb::Options::getThreadCount() const
    {
        return threadCount;
    }

    bool GPUdb::Options::getUseSnappy() const
    {
        return useSnappy;
    }

    std::string GPUdb::Options::getUserAuth() const
    {
        return userAuth;
    }

    std::string GPUdb::Options::getUsername() const
    {
        return username;
    }

    GPUdb::Options& GPUdb::Options::setExecutor(const avro::ExecutorPtr value)
    {
        executor = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setPassword(const std::string& value)
    {
        password = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setThreadCount(const size_t value)
    {
        if (value == 0)
        {
            throw std::invalid_argument("Thread count must be greater than zero.");
        }

        threadCount = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setUseSnappy(const bool value)
    {
        useSnappy = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setUserAuth(const std::string& value)
    {
        userAuth = value;
        return *this;
    }

    GPUdb::Options& GPUdb::Options::setUsername(const std::string& value)
    {
        username = value;
        return *this;
    }

    #include "GPUdbFunctions.cpp"
}
