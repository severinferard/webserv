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

void                Server::send_file(std::string path)
{
    Response response;

    _responses.push_back(response);
}

void                Server::handle_request(Request *request)
{
    send(request->getFd(), request->getPath().c_str(), request->getPath().size(), 0);
    close(request->getFd());
}