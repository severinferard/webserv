#ifndef REQUEST_HPP
#define REQUEST_HPP

# include <iostream>
# include "Server.hpp"
# include <string>

class Server;



class Request
{
    private:
        Server *_server;
        int         _fd;
        std::string _path;
    
    public:
        Request();
        Request(Server *server, int fd, std::string path);
        ~Request();

        Server *getServer(void);
        std::string getPath(void);
        int         getFd(void);
};


#endif /* REQUEST_HPP */
