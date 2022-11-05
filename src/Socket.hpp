#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "Server.hpp"
// # include "Client.hpp"

class Server;
class ListenOperation;
class Client;
class WebservCore;

class Socket
{
private:
    std::string _host;
    uint32_t _port;
    int _fd;
    std::vector<Server *> _servers;

public:
    Socket();
    Socket(std::string host, uint32_t port);
    Socket(std::string host, uint32_t port, int fd);
    ~Socket();

    int listen(int backlog = 5);
    void add_server(Server *server);
    Client *acceptConnection(WebservCore *core) const;

    std::string get_host(void) const;
    uint32_t get_port(void) const;
    int get_fd(void) const;
    const std::vector<Server *> *get_servers(void) const;
};

class ConnectionResetByPeerException : public std::runtime_error
{
private:
public:
    ConnectionResetByPeerException() : runtime_error("Connection closed or reset by peer"){};
    ~ConnectionResetByPeerException() throw() {}
};

#endif /* SOCKET_HPP */
