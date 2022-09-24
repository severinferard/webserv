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
#include "./Parser.hpp"
#include "./Operation.hpp"
#include "./Socket.hpp"
#include "Server.hpp"

class WebservCore {

private:
    std::vector<pollfd>             _pollfds;
    std::map<int, OperationBase *>  _operations;
    std::vector<Server>             _servers;
    // int                             _epoll_fd;


    void add_op(OperationBase *op, short events);
    void delete_op(OperationBase *op);
    
public:
    WebservCore();
    ~WebservCore();

    void setup(std::vector<server_config_t> config);
    void run(void);
    

};

#endif /* CORE_HPP */
