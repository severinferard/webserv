#include "Server.hpp"

Server::Server(server_config_t config):
    config(config),
    server_names(config.server_names),
    listen_on(config.listen_on),
    root(config.root),
    indexes(config.index),
    locations(config.locations),
    error_pages(config.error_pages),
    client_max_body_size(config.client_max_body_size),
    autoindex(config.autoindex)
{
}

Server::~Server()
{
}

const location_t          *Server::findLocation(std::string uri)
{
    std::vector<std::string> uriParsed = splitstr(uri, "/");

    for (std::vector<location_t>::const_iterator loc_it = config.locations.begin(); loc_it != config.locations.end(); loc_it++)
    {
        // std::cout << "testing: " << loc_it->path << std::endl;
        std::vector<std::string> locParsed = splitstr(loc_it->path, "/");

        // Ignore if the URI contains less fields than the location path.
        if (uriParsed.size() < locParsed.size())
            continue;
        //Ignore if the location is marked as strict and the URI contains more fields.
        if (loc_it->modifier == PATH_STRICT && uriParsed.size() != locParsed.size())
            continue;
        
        if (loc_it->modifier == PATH_NO_MODIFIDER)
        {
            // Compare each field up to the last one.
            unsigned int i = 0;
            while (i < locParsed.size() -  1)
            {
                if (uriParsed[i] != locParsed[i] && locParsed[i] != "*")
                    break;
                i++;
            }
            if (locParsed[i] == uriParsed[i] || locParsed[i] == "*")
                return &(*loc_it);
        }
        else if (loc_it->modifier == PATH_ENDWITH)
        {
            if (uri.size() > loc_it->path.size() && uri.substr(uri.size() - loc_it->path.size(), loc_it->path.size()) == loc_it->path)
                return &(*loc_it);
        }
    }
    return NULL;
}
