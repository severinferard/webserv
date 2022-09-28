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
#include "Operation.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "epoll.h"

#define EPOLL_TIMEOUT 10000

class WebservCore {

private:
    std::map<int, OperationBase *>  _operations;
    std::vector<Server>             _servers;
    std::vector<Socket>             _sockets;
    int                             _epoll_fd;


    void                            add_op(OperationBase *op, uint32_t events);
    void                            delete_op(OperationBase *op);
    OperationBase *                 find_op_by_fd(int fd);
    Socket *                        find_socket_on_port(uint32_t port);
    void                            start_listening_sockets(void);
    
public:
    WebservCore();
    ~WebservCore();

    void                            setup(std::vector<server_config_t> config);
    void                            run(void);
    

};

#endif /* CORE_HPP */
