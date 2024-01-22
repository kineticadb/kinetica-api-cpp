#include "gpudb/Http.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>

namespace gpudb
{
    //- TcpSocketBase ----------------------------------------------------------

    template<typename TSocket>
    class TcpSocketBase
    {
    public:
        virtual ~TcpSocketBase()
        {
        }

        void assertNoError()
        {
            if (m_error)
            {
                throw boost::system::system_error(m_error);
            }
        }

        void connect(boost::function<void()> handler)
        {
            setTimeout();
            m_resolver.async_resolve(boost::asio::ip::tcp::resolver::query(m_host, m_service),
                                     boost::bind(&TcpSocketBase::onResolved, this,
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::iterator,
                                                 handler));

            if (m_timeout > 0)
            {
                m_deadline.async_wait(boost::bind(&TcpSocketBase::onDeadline, this));
            }

            m_ioService.run();
        }

        template<typename AsyncReadStream>
        void read(AsyncReadStream& stream, boost::function<void(bool)> handler)
        {
            setTimeout();
            boost::asio::async_read(getSocket(), stream,
                                    boost::bind(&TcpSocketBase::onRead, this,
                                                boost::asio::placeholders::error,
                                                handler));
        }

        template<typename AsyncReadStream, typename CompletionCondition>
        void read(AsyncReadStream& stream, CompletionCondition condition, boost::function<void(bool)> handler)
        {
            setTimeout();
            boost::asio::async_read(getSocket(), stream, condition,
                                    boost::bind(&TcpSocketBase::onRead, this,
                                                boost::asio::placeholders::error,
                                                handler));
        }

        template<typename AsyncReadStream, typename Delimiter>
        void readUntil(AsyncReadStream& stream, const Delimiter& delimiter, boost::function<void(bool)> handler)
        {
            setTimeout();
            boost::asio::async_read_until(getSocket(), stream, delimiter,
                                          boost::bind(&TcpSocketBase::onRead, this,
                                                      boost::asio::placeholders::error,
                                                      handler));
        }

        template<typename AsyncWriteStream>
        void write(AsyncWriteStream& stream, boost::function<void()> handler)
        {
            setTimeout();
            boost::asio::async_write(getSocket(), stream,
                                     boost::bind(&TcpSocketBase::onWrite, this,
                                                 boost::asio::placeholders::error,
                                                 handler));
        }

        void close()
        {
            m_timeout = 0;
            closeSocket();
            m_deadline.cancel();
        }

    protected:
        TcpSocketBase(const std::string& host,
                      const std::string& service,
                      size_t timeout) :
            m_host(host),
            m_service(service),
            m_timeout(timeout),
            m_resolver(m_ioService),
            m_deadline(m_ioService)
        {
        }

        boost::asio::io_service& getIoService()
        {
            return m_ioService;
        }

        virtual void connectSocket(boost::asio::ip::tcp::resolver::iterator iterator,
                                   boost::function<void(const boost::system::error_code&)> handler) = 0;
        virtual TSocket& getSocket() = 0;
        virtual bool isSocketOpen() = 0;
        virtual void closeSocket() = 0;

        void handleConnectEvent(const boost::system::error_code& error,
                                boost::asio::ip::tcp::resolver::iterator iterator,
                                boost::function<void()> finalHandler,
                                boost::function<void()> nextHandler)
        {
            if (!isSocketOpen())
            {
                if (++iterator == boost::asio::ip::tcp::resolver::iterator())
                {
                    m_error = boost::asio::error::timed_out;
                    m_deadline.cancel();
                }
                else
                {
                    doConnect(iterator, finalHandler);
                }
            }
            else if (error)
            {
                closeSocket();

                if (++iterator == boost::asio::ip::tcp::resolver::iterator())
                {
                    checkError(error);
                }
                else
                {
                    doConnect(iterator, finalHandler);
                }
            }
            else
            {
                nextHandler();
            }
        }

        virtual void onConnected(const boost::system::error_code& error,
                                 boost::asio::ip::tcp::resolver::iterator iterator,
                                 boost::function<void()> handler) = 0;

    private:
        std::string m_host;
        std::string m_service;
        size_t m_timeout;
        boost::asio::io_service m_ioService;
        boost::asio::ip::tcp::resolver m_resolver;
        boost::asio::deadline_timer m_deadline;
        boost::system::error_code m_error;

        void setTimeout()
        {
            if (m_timeout > 0)
            {
                m_deadline.expires_from_now(boost::posix_time::milliseconds(m_timeout));
            }
        }

        bool checkError(const boost::system::error_code& error)
        {
            if (error)
            {
                m_error = error;
                closeSocket();
                m_deadline.cancel();
                return true;
            }
            else
            {
                return false;
            }
        }

        void onResolved(const boost::system::error_code& error,
                        boost::asio::ip::tcp::resolver::iterator iterator,
                        boost::function<void()> handler)
        {
            if (checkError(error))
            {
                return;
            }

            doConnect(iterator, handler);
        }

        void doConnect(boost::asio::ip::tcp::resolver::iterator iterator,
                       boost::function<void()> handler)
        {
            setTimeout();
            connectSocket(iterator,
                          boost::bind(&TcpSocketBase::onConnected, this,
                                      boost::asio::placeholders::error,
                                      iterator, handler));
        }

        void onRead(const boost::system::error_code& error,
                    boost::function<void(bool)> handler)
        {
            if (!isSocketOpen())
            {
                m_error = boost::asio::error::timed_out;
                m_deadline.cancel();
            }
            else if (error == boost::asio::error::eof)
            {
                handler(true);
            }
            else if (checkError(error))
            {
                return;
            }
            else
            {
                handler(false);
            }
        }

        void onWrite(const boost::system::error_code& error,
                     boost::function<void()> handler)
        {
            if (!isSocketOpen())
            {
                m_error = boost::asio::error::timed_out;
                m_deadline.cancel();
            }
            else if (checkError(error))
            {
                return;
            }
            else
            {
                handler();
            }
        }

        void onDeadline()
        {
            if (m_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
            {
                m_resolver.cancel();
                closeSocket();
                m_deadline.expires_at(boost::posix_time::pos_infin);
            }

            if (!m_error && m_timeout > 0)
            {
                m_deadline.async_wait(boost::bind(&TcpSocketBase::onDeadline, this));
            }
        }
    };

    //- TcpSocket --------------------------------------------------------------

    class TcpSocket : public TcpSocketBase<boost::asio::ip::tcp::socket>
    {
    public:
        TcpSocket(const std::string& host,
                  const std::string& service,
                  size_t timeout) :
            TcpSocketBase(host, service, timeout),
            m_socket(getIoService())
        {
        }

    protected:
        virtual void connectSocket(boost::asio::ip::tcp::resolver::iterator iterator,
                                   boost::function<void(const boost::system::error_code&)> handler)
        {
            m_socket.async_connect(iterator->endpoint(), handler);
        }

        virtual boost::asio::ip::tcp::socket& getSocket()
        {
            return m_socket;
        }

        virtual bool isSocketOpen()
        {
            return m_socket.is_open();
        }

        virtual void closeSocket()
        {
            m_socket.close();
        }

        virtual void onConnected(const boost::system::error_code& error,
                                 boost::asio::ip::tcp::resolver::iterator iterator,
                                 boost::function<void()> handler)
        {
            handleConnectEvent(error, iterator, handler, handler);
        };

    private:
        boost::asio::ip::tcp::socket m_socket;
    };

    //- SslTcpSocket -----------------------------------------------------------

    #ifndef GPUDB_NO_HTTPS
    class SslTcpSocket : public TcpSocketBase<boost::asio::ssl::stream<boost::asio::ip::tcp::socket> >
    {
    public:
        SslTcpSocket(boost::asio::ssl::context& sslContext,
                     const std::string& host,
                     const std::string& service,
                     size_t timeout) :
            TcpSocketBase(host, service, timeout),
            m_socket(getIoService(), sslContext)
        {
        }

    protected:
        virtual void connectSocket(boost::asio::ip::tcp::resolver::iterator iterator,
                                   boost::function<void(const boost::system::error_code&)> handler)
        {
            m_socket.lowest_layer().async_connect(iterator->endpoint(), handler);
        }

        virtual boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& getSocket()
        {
            return m_socket;
        }

        virtual bool isSocketOpen()
        {
            return m_socket.lowest_layer().is_open();
        }

        virtual void closeSocket()
        {
            m_socket.lowest_layer().close();
        }

        virtual void onConnected(const boost::system::error_code& error,
                                 boost::asio::ip::tcp::resolver::iterator iterator,
                                 boost::function<void()> handler)
        {
            handleConnectEvent(error, iterator, handler,
                               boost::bind(&SslTcpSocket::handshake, this,
                                           iterator, handler));
        }

    private:
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket;

        void handshake(boost::asio::ip::tcp::resolver::iterator iterator,
                       boost::function<void()> handler)
        {
            m_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                     boost::bind(&SslTcpSocket::onHandshake, this,
                                                 boost::asio::placeholders::error,
                                                 iterator, handler));
        }

        void onHandshake(const boost::system::error_code& error,
                         boost::asio::ip::tcp::resolver::iterator iterator,
                         boost::function<void()> handler)
        {
            handleConnectEvent(error, iterator, handler, handler);
        }
    };
    #endif

    //- HttpUrl ----------------------------------------------------------------

    const std::string HttpUrl::HTTP("http");
    const std::string HttpUrl::HTTPS("https");

    HttpUrl::HttpUrl() :
        m_url("http://127.0.0.1/"),
        m_secure(false),
        m_host("127.0.0.1"),
        m_port(80),
        m_path("/")
    {
    }

    HttpUrl::HttpUrl(const char* url)
    {
        init(url);
    }

    HttpUrl::HttpUrl(const std::string& url)
    {
        init(url);
    }

    HttpUrl::HttpUrl(const std::string& protocol,
                     const std::string& host,
                     uint16_t port,
                     const std::string& path,
                     const std::string& query) :
        m_host(host),
        m_port(port),
        m_path(path),
        m_query(query)
    {
        if (protocol == "http")
        {
            m_secure = false;
        }
        else if (protocol == "https")
        {
            m_secure = true;
        }
        else
        {
            throw std::invalid_argument("Invalid protocol specified.");
        }

        if (host.empty())
        {
            throw std::invalid_argument("Invalid host specified.");
        }

        for (size_t i = 0; i < host.length(); ++i)
        {
            if (host[i] == ':' || host[i] == '/')
            {
                throw std::invalid_argument("Invalid host specified.");
            }
        }

        if (port == 0)
        {
            throw std::invalid_argument("Invalid port specified.");
        }

        for (size_t i = 0; i < path.length(); ++i)
        {
            if (path[i] == '?')
            {
                throw std::invalid_argument("Invalid path specified.");
            }
        }

        if (path.empty())
        {
            m_path = "/";
        }

        createUrl();
    }

    HttpUrl::HttpUrl(const HttpUrl& baseUrl,
                     const std::string& path,
                     const std::string& query) :
        m_secure(baseUrl.m_secure),
        m_host(baseUrl.m_host),
        m_port(baseUrl.m_port),
        m_path(baseUrl.m_path),
        m_query(query)
    {
        if (!path.empty())
        {
            if (m_path[m_path.length() - 1] == '/' && path[0] == '/')
            {
                m_path.resize(m_path.length() - 1);
            }

            m_path += path;
        }

        createUrl();
    }

    void HttpUrl::createUrl()
    {
        std::stringstream ss;
        ss << (m_secure ? HTTPS : HTTP);
        ss << "://" << m_host;

        if ((m_secure && m_port != 443) || (!m_secure && m_port != 80))
        {
            ss << ":" << m_port;
        }

        ss << m_path;

        if (!m_query.empty())
        {
            ss << "?" << m_query;
        }

        m_url = ss.str();
    }

    const std::string& HttpUrl::getHost() const
    {
        return m_host;
    }

    const std::string& HttpUrl::getPath() const
    {
        return m_path;
    }

    uint16_t HttpUrl::getPort() const
    {
        return m_port;
    }

    const std::string& HttpUrl::getProtocol() const
    {
        return m_secure ? HTTPS : HTTP;
    }

    const std::string& HttpUrl::getQuery() const
    {
        return m_query;
    }

    const std::string& HttpUrl::getUrl() const
    {
        return m_url;
    }

    std::string HttpUrl::getAppendedUrl( const std::string& endpoint ) const
    {
        // Nothing to append!
        if ( endpoint.empty() )
            return m_url;
        
        std::stringstream ss;

        // Put the content of the current URL
        ss << m_url;

        // Append a '/' if the URL does not end in on
        if ( m_url[ m_url.length() - 1 ] != '/' )
        {
            ss << "/";
        }

        // Append the endpoint, discarding any leading '/'
        if ( endpoint[ 0 ] == '/' && endpoint.length() > 1 )
        {
            ss << endpoint.substr( 1 );
        }
        else
        {
            ss << endpoint;
        }

        return ss.str();
    }


    void HttpUrl::init(const std::string& url)
    {
        if ( url.empty() )
        {
            throw std::invalid_argument("Invalid URL specified; empty string given.");
        }

        size_t i = 0;

        std::string protocol;
        protocol.reserve(8);

        for (; i < url.length(); ++i)
        {
            protocol.push_back(url[i]);

            if (i > 1 && url[i - 2] == ':' && url[i - 1] == '/' && url[i] == '/')
            {
                break;
            }
        }

        if (protocol == "http://")
        {
            m_secure = false;
        }
        else if (protocol == "https://")
        {
            m_secure = true;
        }
        else
        {
            throw std::invalid_argument("Invalid URL specified: '" + url + "'");
        }

        m_host.reserve(url.length() - i - 1);
        bool portSpecified = false;

        for (++i; i < url.length(); ++i)
        {
            if (url[i] == ':')
            {
                portSpecified = true;
                break;
            }
            else if (url[i] == '/' || url[i] == '?')
            {
                portSpecified = false;
                break;
            }

            m_host.push_back(url[i]);
        }

        if (m_host.empty())
        {
            throw std::invalid_argument("Invalid URL specified.");
        }

        if (portSpecified)
        {
            std::string port;
            port.reserve(5);

            for (++i; i < url.length(); ++i)
            {
                if (url[i] == '/')
                {
                    break;
                }
                else if (url[i] < '0' || url[i] > '9' || port.length() > 5)
                {
                    throw std::invalid_argument("Invalid URL specified.");
                }

                port.push_back(url[i]);
            }

            try
            {
                m_port = boost::lexical_cast<uint16_t>(port);
            }
            catch (boost::bad_lexical_cast&)
            {
                throw std::invalid_argument("Invalid URL specified.");
            }

            if (m_port == 0)
            {
                throw std::invalid_argument("Invalid URL specified.");
            }
        }
        else if (m_secure)
        {
            m_port = 443;
        }
        else
        {
            m_port = 80;
        }

        m_path.reserve(url.length() - i);

        for (; i < url.length(); ++i)
        {
            if (url[i] == '?')
            {
                break;
            }

            m_path.push_back(url[i]);
        }

        if (m_path.empty())
        {
            m_path = "/";
        }

        if (++i < url.length())
        {
            m_query = url.substr(i);
        }

        createUrl();
    }

    bool HttpUrl::isSecure() const
    {
        return m_secure;
    }

    HttpUrl::operator std::string() const
    {
        return m_url;
    }

    std::ostream& operator <<(std::ostream& stream, const HttpUrl& value)
    {
        return stream << (std::string)value;
    }

    // Overloaded comparison operator
    bool operator ==(const HttpUrl &lhs, const HttpUrl &rhs)
    {
        return ( (lhs.getUrl().compare( rhs.getUrl() ) == 0)
                 && ( lhs.isSecure() == rhs.isSecure() ) );
    }

    bool operator !=(const HttpUrl &lhs, const HttpUrl &rhs)
    {
        return !(lhs == rhs);
    }

    //- HttpConnection----------------------------------------------------------

    template<typename TSocket>
    class HttpConnection
    {
    public:
        HttpConnection(TSocket& socket,
                       HttpRequest& request,
                       HttpResponse& response) :
            m_socket(socket),
            m_request(request),
            m_response(response)
        {
        }

        void send()
        {
            m_socket.connect(boost::bind(&HttpConnection::onConnect, this));
            m_socket.assertNoError();

            if (!m_error.empty())
            {
                throw std::runtime_error(m_error);
            }
        }

    private:
        TSocket& m_socket;
        HttpRequest& m_request;
        HttpResponse& m_response;
        boost::asio::streambuf m_responseStream;
        std::string m_error;

        void onConnect()
        {
            boost::asio::streambuf headers;
            std::ostream headerStream(&headers);

            const HttpUrl& url = m_request.m_url;
            headerStream << m_request.m_requestMethod << " " << url.getPath();

            if (!url.getQuery().empty())
            {
                headerStream << "?" << url.getQuery();
            }

            headerStream << " HTTP/1.0\r\n";

            std::string host = url.getHost() + ":" + std::to_string(url.getPort());
            // This is needed according to the RFC, but the Java API sends it, so commenting it for now
            //size_t found = host.find_first_not_of("0123456789.:");
            //if (found == std::string::npos) // Just an IP address
            //    host = ""; // Send an empty host if not a host name (per spec)
            headerStream << "Host: " << host << "\r\n";

            // These may be needed for HTTP/1.1
            //headerStream << "Accept: */*\r\n";
            //headerStream << "User-Agent: C/1.0\r\n";
            //headerStream << "Transfer-Encoding: identity\r\n";

            const std::map<std::string, std::string>& requestHeaders = m_request.m_requestHeaders;

            for (std::map<std::string, std::string>::const_iterator it = requestHeaders.begin();
                 it != requestHeaders.end(); ++it)
            {
                headerStream << it->first << ": " << it->second << "\r\n";
            }

            // This may be needed for HTTP/1.1
            //headerStream << "Connection: close\r\n";
            headerStream << "\r\n";
            std::vector<boost::asio::const_buffer> request;
            boost::asio::streambuf::const_buffers_type headerBuffers = headers.data();
            request.insert(request.end(), headerBuffers.begin(), headerBuffers.end());

            const void* requestBody;
            size_t requestBodyLength;
            m_request.read(requestBody, requestBodyLength);

            if (requestBodyLength > 0)
            {
                request.push_back(boost::asio::buffer(requestBody, requestBodyLength));
            }

            m_socket.write(request, boost::bind(&HttpConnection::onWrite, this));
        }

        void onWrite()
        {
            m_socket.readUntil(m_responseStream, "\r\n\r\n",
                               boost::bind(&HttpConnection::onReadHeaders, this, _1));
        }

        void onReadHeaders(bool eof)
        {
            if (eof)
            {
                m_error = "Invalid response from server.";
                m_socket.close();
                return;
            }

            std::istream responseStream(&m_responseStream);
            std::string header;
            std::getline(responseStream, header);

            if (!responseStream || header.find("HTTP/") != 0)
            {
                m_error = "No HTTP version in response.";
                m_socket.close();
                return;
            }

            size_t space = header.find(' ');

            if (space == std::string::npos || space + 4 >= header.length())
            {
                m_error = "No HTTP status code in response.";
                m_socket.close();
                return;
            }

            m_response.m_responseVersion = header.substr(5, space - 5);
            std::string statusCode = header.substr(space + 1, 3);

            if (statusCode.length() != 3)
            {
                m_error = "Invalid HTTP status code in response.";
                m_socket.close();
                return;
            }

            try
            {
                m_response.m_responseCode = boost::lexical_cast<unsigned int>(statusCode);
            }
            catch (boost::bad_lexical_cast&)
            {
                m_error = "Invalid HTTP status code in response.";
                m_socket.close();
                return;
            }

            m_response.m_responseMessage = header.substr(space + 4);
            boost::trim(m_response.m_responseMessage);

            std::map<std::string, std::string>& responseHeaders = m_response.m_responseHeaders;

            while (responseStream)
            {
                std::getline(responseStream, header);

                if (header.empty() || header == "\r")
                {
                    break;
                }

                size_t colon = header.find(':');

                if (colon == std::string::npos)
                {
                    m_error = "Invalid response from server.";
                    m_socket.close();
                    return;
                }

                std::string key = header.substr(0, colon);
                std::string value = header.substr(colon + 1);
                boost::trim(value);

                std::map<std::string, std::string>::iterator it = responseHeaders.find(key);

                if (it == responseHeaders.end())
                {
                    responseHeaders[key] = value;
                }
                else
                {
                    it->second += ", " + value;
                }
            }

            consume();
            m_socket.read(m_responseStream, boost::asio::transfer_at_least(1),
                          boost::bind(&HttpConnection::onReadBody, this, _1));
        }

        void onReadBody(bool eof)
        {
            if (eof)
            {
                m_socket.close();
                return;
            }

            consume();
            m_socket.read(m_responseStream, boost::asio::transfer_at_least(1),
                          boost::bind(&HttpConnection::onReadBody, this, _1));
        }

        void consume()
        {
            boost::asio::streambuf::const_buffers_type data = m_responseStream.data();
            size_t totalRead = 0;

            for (boost::asio::streambuf::const_buffers_type::const_iterator it = data.begin();
                 it != data.end(); ++it)
            {
                const void* buffer = boost::asio::buffer_cast<const void*>(*it);
                size_t read = boost::asio::buffer_size(*it);
                m_response.write(buffer, read);
                totalRead += read;
            }

            m_responseStream.consume(totalRead);
        }
    };

    //- HttpRequest ------------------------------------------------------------

    const std::string HttpRequest::GET("GET");
    const std::string HttpRequest::HEAD("HEAD");
    const std::string HttpRequest::POST("POST");

    HttpRequest::HttpRequest() :
        #ifndef GPUDB_NO_HTTPS
        m_sslContext(NULL),
        m_bypassSslCertCheck( false ),
        #endif

        m_requestMethod(GET),
        m_timeout(0)
    {
    }

    HttpRequest::HttpRequest(const HttpUrl& url) :
        #ifndef GPUDB_NO_HTTPS
        m_sslContext(NULL),
        m_bypassSslCertCheck( false ),
        #endif

        m_url(url),
        m_requestMethod(GET),
        m_timeout(0)
    {
    }

    void HttpRequest::addRequestHeader(const std::string& key, const std::string& value)
    {
        std::map<std::string, std::string>::iterator it = m_requestHeaders.find(key);

        if (it == m_requestHeaders.end())
        {
            m_requestHeaders[key] = value;
        }
        else
        {
            it->second += ", " + value;
        }
    }

    const std::string& HttpRequest::getRequestHeader(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = m_requestHeaders.find(key);

        if (it == m_requestHeaders.end())
        {
            throw std::out_of_range("Header " + key + " does not exist.");
        }
        else
        {
            return it->second;
        }
    }

    std::map<std::string, std::string>& HttpRequest::getRequestHeaders()
    {
        return m_requestHeaders;
    }

    const std::string& HttpRequest::getRequestMethod() const
    {
        return m_requestMethod;
    }

    #ifndef GPUDB_NO_HTTPS
    boost::asio::ssl::context* HttpRequest::getSslContext()
    {
        return m_sslContext;
    }
    #endif

    size_t HttpRequest::getTimeout() const
    {
        return m_timeout;
    }

    void HttpRequest::read(const void*& data, size_t& length) const
    {
        data = NULL;
        length = 0;
    }

    const HttpUrl& HttpRequest::getUrl() const
    {
        return m_url;
    }

    void HttpRequest::send(HttpResponse& response)
    {
        if (m_url.isSecure())
        {
            #ifdef GPUDB_NO_HTTPS
            throw std::runtime_error("HTTPS not supported.");
            #else
            boost::scoped_ptr<boost::asio::ssl::context> tempSslContext;

            if (!m_sslContext)
            {
                tempSslContext.reset(new boost::asio::ssl::context(boost::asio::ssl::context::sslv23));
                tempSslContext->set_default_verify_paths();
                int verifyCert = m_bypassSslCertCheck;
                tempSslContext->set_verify_mode(m_bypassSslCertCheck ? boost::asio::ssl::verify_none : boost::asio::ssl::verify_peer);
                if( !m_bypassSslCertCheck )
                {
                    tempSslContext->set_verify_callback(boost::asio::ssl::rfc2818_verification(m_url.getHost()));
                }
            }

            SslTcpSocket socket(m_sslContext ? *m_sslContext : *tempSslContext,
                                m_url.getHost(),
                                boost::lexical_cast<std::string>(m_url.getPort()),
                                m_timeout);
            HttpConnection<SslTcpSocket> connection(socket, *this, response);
            connection.send();
            #endif
        }
        else
        {
            TcpSocket socket(m_url.getHost(), boost::lexical_cast<std::string>(m_url.getPort()), m_timeout);
            HttpConnection<TcpSocket> connection(socket, *this, response);
            connection.send();
        }
    }

    void HttpRequest::setRequestMethod(const std::string& method)
    {
        m_requestMethod = method;
    }

    #ifndef GPUDB_NO_HTTPS
    void HttpRequest::setSslContext(boost::asio::ssl::context* sslContext)
    {
        m_sslContext = sslContext;
    }
    void HttpRequest::setBypassSslCertCheck(const bool value)
    {
        m_bypassSslCertCheck = value;
    }
    #endif

    void HttpRequest::setTimeout(size_t timeout)
    {
        m_timeout = timeout;
    }

    void HttpRequest::setUrl(const HttpUrl& url)
    {
        m_url = url;
    }

    //- BinaryHttpRequest ------------------------------------------------------

    BinaryHttpRequest::BinaryHttpRequest() :
        HttpRequest()
    {
    }

    BinaryHttpRequest::BinaryHttpRequest(const HttpUrl& url) :
        HttpRequest(url)
    {
    }

    const std::vector<uint8_t>* BinaryHttpRequest::getRequestBody()
    {
        return m_requestBody;
    }

    void BinaryHttpRequest::setRequestBody(const std::vector<uint8_t>* requestBody)
    {
        m_requestBody = requestBody;
    }

    void BinaryHttpRequest::read(const void*& data, size_t& length) const
    {
        data = &(*m_requestBody)[0];
        length = m_requestBody->size();
    }

    //- StringHttpRequest ------------------------------------------------------

    StringHttpRequest::StringHttpRequest() :
        HttpRequest()
    {
    }

    StringHttpRequest::StringHttpRequest(const HttpUrl& url) :
        HttpRequest(url)
    {
    }

    const std::string* StringHttpRequest::getRequestBody()
    {
        return m_requestBody;
    }

    void StringHttpRequest::setRequestBody(const std::string* requestBody)
    {
        m_requestBody = requestBody;
    }

    void StringHttpRequest::read(const void*& data, size_t& length) const
    {
        data = &(*m_requestBody)[0];
        length = m_requestBody->length();
    }

    //- HttpResponse -----------------------------------------------------------

    HttpResponse::HttpResponse() :
        m_responseCode(0)
    {
    }

    HttpResponse::~HttpResponse()
    {
    }

    unsigned int HttpResponse::getResponseCode() const
    {
        return m_responseCode;
    }

    const std::string& HttpResponse::getResponseHeader(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = m_responseHeaders.find(key);

        if (it == m_responseHeaders.end())
        {
            throw std::out_of_range("Header " + key + " does not exist.");
        }
        else
        {
            return it->second;
        }
    }

    const std::map<std::string, std::string>& HttpResponse::getResponseHeaders() const
    {
        return m_responseHeaders;
    }

    const std::string& HttpResponse::getResponseMessage() const
    {
        return m_responseMessage;
    }

    const std::string& HttpResponse::getResponseVersion() const
    {
        return m_responseVersion;
    }

    void HttpResponse::write(const void*, size_t)
    {
    }

    //- BinaryHttpResponse -----------------------------------------------------

    const std::vector<uint8_t>& BinaryHttpResponse::getResponseBody() const
    {
        return m_responseBody;
    }

    void BinaryHttpResponse::write(const void* data, size_t length)
    {
        m_responseBody.insert(m_responseBody.end(), (const uint8_t*)data, (const uint8_t*)data + length);
    }

    //- StringHttpResponse -----------------------------------------------------

    const std::string& StringHttpResponse::getResponseBody() const
    {
        return m_responseBody;
    }

    void StringHttpResponse::write(const void* data, size_t length)
    {
        m_responseBody.insert(m_responseBody.end(), (const char*)data, (const char*)data + length);
    }
}
