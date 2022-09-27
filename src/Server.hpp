#ifndef SERVER_HPP
#define SERVER_HPP

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

# include "config.hpp"
# include "Operation.hpp"
# include "Socket.hpp"

class Socket;


class Server {
private:
    server_config_t         _config;

public:
    Server(server_config_t config);
    ~Server();
    
    server_config_t     &get_config(void);
    
};

#endif /* SERVER_HPP */
