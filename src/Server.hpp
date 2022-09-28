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
# include "Response.hpp"
# include "Request.hpp"

class Socket;
class Request;
typedef struct server_config_s server_config_t;

class Server {
private:
    server_config_t         _config;
    std::vector<Response>   _responses;

public:
    Server(server_config_t config);
    ~Server();
    
    server_config_t     &get_config(void);
    void                send_file(std::string path);
    void                handle_request(Request *request);
    
};

#endif /* SERVER_HPP */
