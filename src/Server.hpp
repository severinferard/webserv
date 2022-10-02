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

public:
    Server(server_config_t config);
    ~Server();
    
    const server_config_t               config;
    const std::vector<std::string>      server_names;
    const std::vector<host_port_t>      listen_on;
    const std::string                   root;
    const std::vector<std::string>      indexes;
    const std::vector<location_t>       locations;
    const std::map<int, error_page_t>   error_pages;
    const std::vector<std::string>      allowed_methods;
    const int                           client_max_body_size;
    const int                           autoindex;

    const location_t          *findLocation(std::string uri);
    
};

#endif /* SERVER_HPP */
