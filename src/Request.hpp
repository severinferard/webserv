#ifndef REQUEST_HPP
#define REQUEST_HPP

# include <iostream>
# include "Server.hpp"
# include <string>
# include <cstdio>
# include "utils.hpp"
# include <iomanip>
# include <algorithm>
# include "utils.hpp"
# include "Socket.hpp"
# include "Logger.hpp"
# include "HttpError.hpp"

# define MAX_HEADER_SIZE 1024

class Server;
class Socket;

typedef struct s_chunk {
    size_t  start;
    size_t  size;
    bool    hasSize;
}               t_chunk;

class Request
{
    private:
        int                                 _fd;
        int                                 __log_fd;
        std::string                         _method;
        std::string			                _uri;
        std::string                         _route;
        std::string                         _queryString;
        std::string			                _version;
        const Socket                       *_socket;
        Server                             *_server;
        location_t                          _location;
        bool                                _hasLocation;
        bool                                _headerReceived;
        std::string                         _payload;
        size_t                              _bodyStart;  
        size_t                              _contentLength;    
        bool                                _chunked;
        t_chunk                             _currentChunk;
        std::string                         _userAgent;

        static void                         _addHeader(std::string line, std::map<std::string, std::string> &headers);
        void                                _setHeaders(std::map<std::string, std::string> headers);
        Server *			                findServer(void);
    public:
        std::string			                body;
        std::map<std::string, std::string>  headers;
        bool                                expect_100;
        Request(const Socket *sock, int connection_fd);
        ~Request();

        int                                 parse(void);
        void                                validate(std::vector<std::string>lines, size_t headerLineCount);
        void                                appendToPayload(char *str, size_t size);
        std::string                         getPayload(void) const;
        std::string                         getUri(void) const;
        std::string                         getMethod(void) const;
        std::string                         getVersion(void) const;
        const std::map<std::string, std::string>  &getHeaders(void) const;
        std::string                         getBody(void) const;
        int                                 getFd(void) const;
        std::string                         getRoute(void) const;
        std::string                         getUserAgent(void) const;
        std::string                         getQueryString(void) const;
        location_t                          *getLocation(void) ;
        Server                              *getServer(void) const;
        size_t                              getContentLength(void) const;
};

std::ostream& operator<<(std::ostream& os, Request const& r);


#endif /* REQUEST_HPP */
