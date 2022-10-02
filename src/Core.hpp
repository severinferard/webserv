#ifndef CORE_HPP
#define CORE_HPP

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
#include <sys/poll.h>
#include <map>
#include "Parser.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "epoll.h"

#define EPOLL_TIMEOUT 10000

class Client;
class Socket;
class WebservCore {

private:
    std::vector<Server>             _servers;
    std::vector<Socket>             _sockets;
    std::map<int, Client *>         _clients;
    int                             _epoll_fd;

    Socket *                        _findSocketOnPort(uint32_t port);
    void                            _startListeningSockets(void);
    bool                            _isListeningSocket(int fd);
    Socket *                        _getListeningSocket(int fd);
    Client *                        _findClient(int fd);
    
public:
    WebservCore();
    ~WebservCore();

    void                            setup(std::vector<server_config_t> config);
    void                            run(void);
    void                            registerFd(int fd, uint32_t events, Client *client);
    void                            registerFd(int fd, uint32_t events);
    void                            modifyFd(int fd, uint32_t events);
    void                            unregisterFd(int fd);
    

};

#endif /* CORE_HPP */
