#include "Socket.hpp"
#include "Client.hpp"

Socket::Socket()
{
}

Socket::Socket(std::string host, uint32_t port):
    _host(host),
    _port(port)
{
}

Socket::Socket(std::string host, uint32_t port, int fd):
    _host(host),
    _port(port),
    _fd(fd)
{
}

Socket::~Socket()
{
}

int    Socket::listen(int backlog)
{
    struct in_addr host;
    struct sockaddr_in serv_addr;

    _fd =  socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) 
        throw std::runtime_error("Error opening socket");
    int reuseAddr = 1; 
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

    inet_pton(AF_INET, _host.c_str(), &host);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = host;
    serv_addr.sin_port = htons(_port);

    if (bind(_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        if (errno == EADDRINUSE)
            throw AddressAlreadyInUseException(_port);
        throw std::runtime_error("ERROR on binding");
    }
    Logger::Log(InfoP, "Listening on %s:%u", _host.c_str(), _port);
    ::listen(_fd, backlog);
    return _fd;
}

 void            Socket::add_server(Server * server)
 {
    _servers.push_back(server);
 }

std::string Socket::get_host(void) const
{
    return _host;
}

uint32_t    Socket::get_port(void) const
{
    return _port;
}

int         Socket::get_fd(void) const
{
    return _fd;
}

const std::vector<Server *> *Socket::get_servers(void) const
{
    return &_servers;
}

Client          *Socket::acceptConnection(WebservCore *core) const
{
    struct sockaddr_in  cli_addr;
    socklen_t           cli_len;
    int                 new_fd;

    cli_len = sizeof(cli_addr);
    new_fd = ::accept(_fd, (struct sockaddr *) &cli_addr, &cli_len);

    return new Client(core, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), this, new_fd);
}