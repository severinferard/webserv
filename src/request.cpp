# include "request.hpp"

inline std::ostream &operator<<(std::ostream &os, const http_methods_e &method)
{
    switch (method)
    {
    case GET:
        os << "GET";
        break;
    case HEAD:
        os << "HEAD";
        break;
    case POST:
        os << "POST";
        break;
    case DELETE:
        os << "DELETE";
        break;
    case PUT:
        os << "PUT";
        break;
    };
    return os;
}

Request::Request()
{
}

Request::Request(Server *server, int fd, std::string path):
_server(server),
_fd(fd),
_path(path)
{
}

Request::~Request()
{
}

Server *Request::getServer(void)
{
    return _server;
}

std::string Request::getPath(void)
{
    return _path;
}

int         Request::getFd(void)
{
    return _fd;
}