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
# include "config.hpp"
# include "Operation.hpp"
# include "Response.hpp"
# include "Request.hpp"

class Request;
typedef struct server_config_s server_config_t;

class Server {
private:
    const server_config_t         _config;

public:
    Server(server_config_t config);
    ~Server();
    
    const server_config_t     &get_config(void) const;
    void                send_file(std::string path);
    const location_t          *findLocation(std::string uri);
    // int                resolveAndOpenFile(std::string uri);
    
};

#endif /* SERVER_HPP */
