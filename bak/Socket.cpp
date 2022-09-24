#include "Socket.hpp"

Socket::Socket()
{
}

Socket::Socket(Server *server, std::string host, uint32_t port):
    _server(server),
    _host(host),
    _port(port)
{
}

Socket::Socket(Server *server, std::string host, uint32_t port, int fd):
    _server(server),
    _host(host),
    _port(port),
    _fd(fd)
{
}

Socket::~Socket()
{
}

ListenOperation *Socket::listen(int backlog)
{
    struct in_addr host;
    struct pollfd pfd;
    struct sockaddr_in serv_addr;

    _fd =  socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) 
        throw std::runtime_error("Error opening socket");

    inet_pton(AF_INET, _host.c_str(), &host);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = host;
    serv_addr.sin_port = htons(_port);

    if (bind(_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        throw std::runtime_error("ERROR on binding");
    printf("Listening on %s:%u\n", _host.c_str(), _port);
    ::listen(_fd, backlog);
    printf("FD: %d\n", _fd);
    return new ListenOperation(_server, this);
}

std::string Socket::get_host(void)
{
    return _host;
}

uint32_t    Socket::get_port(void)
{
    return _port;
}

int         Socket::get_fd(void)
{
    return _fd;
}