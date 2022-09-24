#ifndef SOCKET_HPP
#define SOCKET_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/types.h> 
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <poll.h>

# include "./Operation.hpp"
# include "Server.hpp"

class Server;
class ListenOperation;

class Socket {
    public:
        Socket();
        Socket(Server *server, std::string host, uint32_t port);
        Socket(Server *server, std::string host, uint32_t port, int fd);
        ~Socket();

        ListenOperation *listen(int backlog = 5);

        std::string get_host(void);
        uint32_t    get_port(void);
        int         get_fd(void);

    private:
        std::string _host;
        uint32_t    _port;
        int         _fd;
        Server *    _server;
        
};

#endif /* SOCKET_HPP */
