#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <string>
# include "Socket.hpp"

class Client
{
    private:
        std::string _addr;
        int         _port;
        Socket      *_socket;

    public:
        Client();
        ~Client();
};

#endif /* CLIENT_HPP */
