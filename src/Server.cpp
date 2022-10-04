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

#define PRINT_STRING_VECTOR(vector) std::copy(vector.begin(), vector.end(), std::ostream_iterator<std::string>(std::cout, "--")); std::cout << std::endl;

const location_t          *Server::findLocation(std::string uri)
{
    std::vector<std::string> uriParsed = splitstr(uri, "/");
    // printf("trying to match %s\n", uri.c_str());
    // PRINT_STRING_VECTOR(uriParsed);

    // Check exact matches
    for (std::vector<location_t>::const_iterator loc_it = config.locations.begin(); loc_it != config.locations.end(); loc_it++)
    {
        if (loc_it->path == uri)
            return &(*loc_it);
    }

    // Check longest prefix
    std::vector<int> macthesLength;
    for (std::vector<location_t>::const_iterator loc_it = config.locations.begin(); loc_it != config.locations.end(); loc_it++)
    {
        // printf("testing %s\n",  loc_it->path.c_str());
        std::vector<std::string> locPathParsed = splitstr(loc_it->path, "/");

        // Ignore if the location is longer than the uri or if the location is marked as PATH_ENDWITH as it will be handled later.
        if (uriParsed.size() < locPathParsed.size() || loc_it->modifier == PATH_ENDWITH)
        {
            macthesLength.push_back(0);
            continue;
        }
        
        // Check if each field match
        size_t i = 0;
        while (i < locPathParsed.size() && uriParsed[i] == locPathParsed[i])
            i++;
        // If so store the number of fields to choose the longest match
        macthesLength.push_back(i == locPathParsed.size() ? locPathParsed.size() : 0);
    //    printf("matches %ld\n", i == locPathParsed.size() ? locPathParsed.size() : 0);
    }
    // Check if we have a match
    std::vector<int>::iterator maxMatch = std::max_element(macthesLength.begin(),macthesLength.end());
    if (*maxMatch > 0)
        return &(config.locations[maxMatch - macthesLength.begin()]);
    

    // Check Sufixes
    for (std::vector<location_t>::const_iterator loc_it = config.locations.begin(); loc_it != config.locations.end(); loc_it++)
    {
        if (loc_it->modifier == PATH_ENDWITH)
        {
            if (uri.size() > loc_it->path.size() && uri.substr(uri.size() - loc_it->path.size(), loc_it->path.size()) == loc_it->path)
                return &(*loc_it);
        }
    }

    return NULL;

}
