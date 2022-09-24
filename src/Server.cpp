#include "Server.hpp"

Server::Server(server_config_t config)
{
    for (std::vector<host_port_t>::iterator it = config.listen_on.begin(); it < config.listen_on.end(); it++)
    {
        Socket sock(this, it->host, it->port);
        _listening_sockets.push_back(sock);
    }
}

Server::~Server()
{
}

std::vector<Socket> &Server::get_listening_sockets(void)
{
    return _listening_sockets;
}