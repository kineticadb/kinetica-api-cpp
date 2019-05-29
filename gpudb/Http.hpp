#ifndef __GPUDB__HTTP_HPP__
#define __GPUDB__HTTP_HPP__

#ifndef GPUDB_NO_HTTPS
#include <boost/asio/ssl.hpp>
#endif

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace gpudb {
    class HttpUrl
    {
    public:
        static const std::string HTTP;
        static const std::string HTTPS;

        HttpUrl();
        HttpUrl(const char* url);
        HttpUrl(const std::string& url);
        HttpUrl(const std::string& protocol,
                const std::string& host,
                uint16_t port,
                const std::string& path,
                const std::string& query = std::string());
        HttpUrl(const HttpUrl& baseUrl,
                const std::string& path,
                const std::string& query = std::string());

        const std::string& getUrl() const;
        const std::string& getProtocol() const;
        bool isSecure() const;
        const std::string& getHost() const;
        uint16_t getPort() const;
        const std::string& getPath() const;
        const std::string& getQuery() const;

        /// Returns a string with the passed in endpoint appended
        /// to this URL.  Takes care of the '/' separating this URL
        /// and the endpoint (i.e. adds one if needed or gets rid of
        /// one if two are presetnt).
        std::string getAppendedUrl( const std::string& endpoint ) const;

        operator std::string() const;

    private:
        std::string m_url;
        bool m_secure;
        std::string m_host;
        uint16_t m_port;
        std::string m_path;
        std::string m_query;

        void init(const std::string& url);
        void createUrl();
    };

    std::ostream& operator <<(std::ostream& stream, const HttpUrl& value);

    template<typename T>
    class HttpConnection;

    class HttpResponse;

    class HttpRequest
    {
    public:
        static const std::string GET;
        static const std::string HEAD;
        static const std::string POST;

        HttpRequest();
        HttpRequest(const HttpUrl& url);

        #ifndef GPUDB_NO_HTTPS
        boost::asio::ssl::context* getSslContext();
        #endif

        const HttpUrl& getUrl() const;
        const std::string& getRequestMethod() const;
        const std::string& getRequestHeader(const std::string& key) const;
        std::map<std::string, std::string>& getRequestHeaders();
        size_t getTimeout() const;

        #ifndef GPUDB_NO_HTTPS
        void setSslContext(boost::asio::ssl::context* sslContext);
        #endif

        void setUrl(const HttpUrl& url);
        void setRequestMethod(const std::string& method);
        void addRequestHeader(const std::string& key, const std::string& value);
        void setTimeout(const size_t timeout);

        void send(HttpResponse& response);

    protected:
        virtual void read(const void*& data, size_t& length) const;

    private:
        template<typename T>
        friend class HttpConnection;

        #ifndef GPUDB_NO_HTTPS
        boost::asio::ssl::context* m_sslContext;
        #endif

        HttpUrl m_url;
        std::string m_requestMethod;
        std::map<std::string, std::string> m_requestHeaders;
        size_t m_timeout;
    };

    class BinaryHttpRequest : public HttpRequest
    {
    public:
        BinaryHttpRequest();
        BinaryHttpRequest(const HttpUrl& url);

        const std::vector<uint8_t>* getRequestBody();

        void setRequestBody(const std::vector<uint8_t>* requestBody);

    protected:
        virtual void read(const void*& data, size_t& length) const;

    private:
        const std::vector<uint8_t>* m_requestBody;
    };

    class StringHttpRequest : public HttpRequest
    {
    public:
        StringHttpRequest();
        StringHttpRequest(const HttpUrl& url);

        const std::string* getRequestBody();

        void setRequestBody(const std::string* requestBody);

    protected:
        virtual void read(const void*& data, size_t& length) const;

    private:
        const std::string* m_requestBody;
    };

    class HttpResponse
    {
    public:
        HttpResponse();
        virtual ~HttpResponse();

        const std::string& getResponseVersion() const;
        unsigned int getResponseCode() const;
        const std::string& getResponseMessage() const;
        const std::string& getResponseHeader(const std::string& key) const;
        const std::map<std::string, std::string>& getResponseHeaders() const;

    protected:
        virtual void write(const void* data, const size_t length);

    private:
        template<typename T>
        friend class HttpConnection;

        std::string m_responseVersion;
        unsigned int m_responseCode;
        std::string m_responseMessage;
        std::map<std::string, std::string> m_responseHeaders;
    };

    class BinaryHttpResponse : public HttpResponse
    {
    public:
        const std::vector<uint8_t>& getResponseBody() const;

    protected:
        virtual void write(const void* data, const size_t length);

    private:
        std::vector<uint8_t> m_responseBody;
    };

    class StringHttpResponse : public HttpResponse
    {
    public:
        const std::string& getResponseBody() const;

    protected:
        virtual void write(const void* data, const size_t length);

    private:
        std::string m_responseBody;
    };
}

#endif
