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
# include "exceptions.hpp"

class Server;
class ListenOperation;

class Socket {
    private:
        std::string             _host;
        uint32_t                _port;
        int                     _fd;
        std::vector<Server *>   _servers;

    public:
        Socket();
        Socket(std::string host, uint32_t port);
        Socket(std::string host, uint32_t port, int fd);
        ~Socket();

        ListenOperation *listen(int backlog = 5);
        void            add_server(Server * server);

        std::string get_host(void);
        uint32_t    get_port(void);
        int         get_fd(void);
        
};

#endif /* SOCKET_HPP */
