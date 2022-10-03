#ifndef REQUEST_HPP
#define REQUEST_HPP

# include <iostream>
# include "Server.hpp"
# include <string>
# include <cstdio>
# include "utils.hpp"
# include <iomanip>
# include "utils.hpp"

class Server;

class Request
{
    private:
        int                                 _fd;
        std::string                         _payload;
        std::string                         _method;
        std::string			                _uri;
        std::string			                _version;
        std::map<std::string, std::string>  _headers;
        std::string			                _body;

        void _addHeader(std::string line);
    
    public:
        Request();
        Request(int fd, std::string payload);
        ~Request();

        void parse(void);

        std::string                         getPayload(void) const;
        std::string                         getUri(void) const;
        std::string                         getMethod(void) const;
        std::string                         getVersion(void) const;
        const std::map<std::string, std::string>  &getHeaders(void) const;
        std::string                         getBody(void) const;
        int                                 getFd(void) const;
};

std::ostream& operator<<(std::ostream& os, Request const& r);


#endif /* REQUEST_HPP */
