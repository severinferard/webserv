#include "Server.hpp"

Server::Server(server_config_t config): _config(config)
{
}

Server::~Server()
{
}

server_config_t     &Server::get_config(void)
{
    return _config;
}