#include "Server.hpp"

Server::Server(server_config_t config): _config(config)
{
}

Server::~Server()
{
}

const server_config_t     &Server::get_config(void) const
{
    return _config;
}

void                Server::send_file(std::string path)
{

}

const location_t          *Server::findLocation(std::string uri)
{
    std::vector<std::string> uriParsed = splitstr(uri, "/");

    for (std::vector<location_t>::const_iterator loc_it = _config.locations.begin(); loc_it != _config.locations.end(); loc_it++)
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
            for (unsigned int i = 0; i < locParsed.size(); i++)
            {
                if (uriParsed[i] != locParsed[i] && locParsed[i] != "*")
                    break;
            }
            if (locParsed[locParsed.size() - 1] == uriParsed[locParsed.size() - 1] || locParsed[locParsed.size() - 1] == "*")
                return &(*loc_it);
        }
        else if (loc_it->modifier == PATH_ENDWITH)
        {
            if (uri.substr(uri.size() - loc_it->path.size(), loc_it->path.size()) == loc_it->path)
                return &(*loc_it);
        }
    }
    return NULL;
}
