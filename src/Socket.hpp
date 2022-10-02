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

# include "Server.hpp"
# include "Client.hpp"

class Server;
class ListenOperation;
class Client;

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

        int                         listen(int backlog = 5);
        void                        add_server(Server * server);
        Client                      *acceptConnection(void) const;

        std::string                 get_host(void) const;
        uint32_t                    get_port(void) const;
        int                         get_fd(void) const;
        const std::vector<Server *> *get_servers(void) const;
        
};

class ConnectionResetByPeerException : public std::runtime_error {
	private:
		std::string msg;
		int _fd;
	public:
	ConnectionResetByPeerException(const Socket *socket, const int fd) :runtime_error("Connection reset by peer"), _fd(fd) {
		std::ostringstream ss;
		ss << socket->get_host() << ":" << socket->get_port() << "Connection Reset By Peer with fd " << socket->get_fd()  << std::endl;
		msg = ss.str();
	};
	~ConnectionResetByPeerException() throw() {}
	int getFd(void) { return _fd; }
    const char* what() const throw()
	{
		return msg.c_str();
	}
};

#endif /* SOCKET_HPP */
