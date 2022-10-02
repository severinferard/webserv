#include "Client.hpp"
char Client::_buffer[BUFFER_SIZE];

Client::Client(std::string addr, int port, const Socket *socket, int fd):
    _status(STATUS_WAIT_FOR_REQUEST),
    addr(addr),
    port(port),
    socket(socket),
    connection_fd(fd)
{
}

Client::~Client()
{
}

void			Client::_handleGet(void)
{
    std::string filepath;

    // Generate the file path from the configured root
    if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, _request.getUri());
    else
        filepath = joinPath(_server->get_config().root, _request.getUri());
    std::cout << "filepath: " << filepath << std::endl;

    if (uriIsDirectory(filepath))
        {
            _file_fd = -1;
            // Check if the location provides an index that is found in this directory.
            if (_location && !_location->index.empty())
                _file_fd = _findIndex(filepath, _location->index);
            // If no index found on the location level, check if the server provides an index that is found in this directory.
            else if ( !_server->get_config().index.empty())
                _file_fd = _findIndex(filepath, _server->get_config().index);
            // If no index can be found, check if directory listing is enabled
            if (_file_fd < 0 && ((_location && _location->autoindex) || _server->get_config().autoindex) )
            {

                // PROVISOIRE
                std::cout << "AUTOINDEX\n";
                _response.appendToBody("<h1>Autoindex</h1>");
                _response.setStatus(HTTP_STATUS_SUCCESS);
                _status = STATUS_WAIT_TO_SEND;
                _core->modifyFd(connection_fd, EPOLLOUT);
                return;
            }
            // Otherwise the bad fd will be catched downstream and raise a 404.
        }
        else
        {
            _file_fd = ::open(filepath.c_str(), O_RDONLY);
        }
        
        if (_file_fd <= 0)
            throw HttpError(HTTP_STATUS_NOT_FOUND);
        _status = STATUS_WAIT_TO_READ_FILE;
        _response.setStatus(HTTP_STATUS_SUCCESS);
        _core->registerFd(_file_fd, EPOLLIN, this);
}

void			Client::_handlePost(void)
{
    throw HttpError(HTTP_STATUS_NOT_FOUND); // Provisoire
}

void			Client::_handlePut(void)
{
    throw HttpError(HTTP_STATUS_NOT_FOUND); // Provisoire
}

void			Client::_handleDelete(void)
{
    throw HttpError(HTTP_STATUS_NOT_FOUND); // Provisoire
}



int     Client::_findIndex(std::string dir, std::vector<std::string> const &candidates)
{
    int fd;
    std::string path;

    for (std::vector<std::string>::const_iterator it = candidates.begin(); it != candidates.end(); it++)
    {
        path = joinPath(dir, *it);
        if ((fd = ::open(path.c_str(), O_RDONLY)) > 0)
            return fd;
    }
    return -1;
}


Request        Client::readRequest(void)
{
    int ret = recv(connection_fd, _buffer, BUFFER_SIZE, 0);
    if (ret <= 0)
    {
        close(connection_fd);
        if (ret == 0)
            throw ConnectionResetByPeerException(socket, connection_fd);
        else
            throw std::runtime_error("Error reading request");
    }
    else
    {
        _buffer[ret] = 0;
        return Request(connection_fd, _buffer);
    }
}

bool	Client::readFileToResponseBody(void)
{
    int ret = read(_file_fd, _buffer, BUFFER_SIZE - 1);
    _buffer[ret] = 0; 
    if (ret > 0)
        _response.appendToBody(_buffer);
    if (ret < BUFFER_SIZE - 1 ||  ret <= 0)
    {
        close(_file_fd);
        return true;
    }
    return false;
}

void			Client::_onReadToReadRequest(void)
{
        std::string method;
        std::vector<std::string> allowedMethods;

        // Read and parse the request
        _request = readRequest();
        _request.parse();
        std::cout << _request << std::endl;
        // Find the server using the entry socket and server_name
        _server = findServer();
        std::cout << "root: " << _server->get_config().root << std::endl;

        // Checking if the current route match a location block
        _location = const_cast<location_t *>(_server->findLocation(_request.getUri()));
        std::cout << "location: " << (_location ? _location->path : "No location") << std::endl;

        // Checking if the HTTP methods are restricted for this route
        method = _request.getMethod();
        allowedMethods = _location && !_location->allowed_methods.empty()
            ? _location->allowed_methods
            : _server->get_config().allowed_methods;
        
        // If so, return 405 if the method is not allowed
        if (!allowedMethods.empty() && std::find(allowedMethods.begin(), allowedMethods.end(), method) == allowedMethods.end())
            throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);
        
        if (method == "GET")
            _handleGet();
        else if (method == "POST")
            _handlePost();
        else if (method == "PUT")
            _handlePut();
        else if (method == "DELETE")
            _handleDelete();
        
}

void			Client::_onReadToReadFile(void)
{
    bool empty = readFileToResponseBody();
    if (empty)
    {
        _status = STATUS_WAIT_TO_SEND;
        _core->unregisterFd(_file_fd);
        _core->modifyFd(connection_fd, EPOLLOUT);
    }
}

void			Client::_onReadToSend(void)
{
    _response.send(connection_fd);
    close(connection_fd);
    _core->unregisterFd(connection_fd);
}

void        Client::resume(void)
{
    try
    {
        switch (_status)
        {
        case STATUS_WAIT_FOR_REQUEST:
            _onReadToReadRequest();
            break;
        
        case STATUS_WAIT_TO_READ_FILE:
            _onReadToReadFile();
            break;
        
        case STATUS_WAIT_TO_SEND:
            _onReadToSend();
            break;

        default:
            break;
        }
    }
    catch(const HttpError& e)
    {
        error_page_t errorPage;

        std::cerr << "HTTP Error: " << e.status << " " << Response::HTTP_STATUS[e.status] << '\n';
        if (_location && hasKey<int, error_page_t>(_location->error_pages, e.status))
            errorPage = _location->error_pages.at(e.status);
        else
            errorPage =  _server->get_config().error_pages.at(e.status);
        printf("error page %s\n", errorPage.path.c_str());
        _response.setStatus(errorPage.code);
        _file_fd = ::open(errorPage.path.c_str(), O_RDONLY);
        _core->registerFd(_file_fd, EPOLLIN, this);
        _status = STATUS_WAIT_TO_READ_FILE;
    }
    catch (ConnectionResetByPeerException &e)
    {
        std::cout << e.what() << std::endl;
    }
    
}

Server *			Client::findServer(void)
{
    std::vector<Server *> candidates = *socket->get_servers();
    std::vector<Server *> results;

    // If there is only 1 server, return it.
    if (candidates.size() == 1)
        return candidates.front();

    // 1. Listen specificity - check if both the IP and port are set
    for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
    {
        const server_config_t config = (*it)->get_config();
        for (std::vector<host_port_t>::const_iterator listen_it = config.listen_on.begin(); listen_it < config.listen_on.end(); listen_it++)
        {
            // Check if this "listen" directive fits the current client socket and if so check wether both IP and port are set.
            if (listen_it->host == socket->get_host() && listen_it->port == socket->get_port() && (listen_it->hostIsSet && listen_it->portIsSet))
                results.push_back(*it);
        }
    }
    if (results.empty())
    {
        // 1.5. Listen specificity - check if the address is defined
        for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
        {
            const server_config_t config = (*it)->get_config();
            for (std::vector<host_port_t>::const_iterator listen_it = config.listen_on.begin(); listen_it < config.listen_on.end(); listen_it++)
            {
                // Check if this "listen" directive fits the current client socket and if so check wether both IP and port are set.
                if (listen_it->host == socket->get_host() && listen_it->port == socket->get_port() && listen_it->hostIsSet)
                    results.push_back(*it);
            }
        }
    }

    if (results.size() == 1)
        return results[0];
    if (results.size() > 1)
        candidates = results;


    std::map<std::string, std::string>::const_iterator host_it = _request.getHeaders().find("host");
    if (host_it == _request.getHeaders().end())
        host_it = _request.getHeaders().find("Host");
    // If the request doesnt contain a Host header, return the first candidate.
    if (host_it == _request.getHeaders().end())
        return candidates[0];
    std::string hostName = host_it->second;

    // 2. server_name exact match
    for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
    {
        const server_config_t config = (*it)->get_config();
        // Check if we can find the hostname in the server's server_name vector.
        // return directly if an exact match is found as nginx uses the first match.
        if (std::find(config.server_names.begin(), config.server_names.end(), hostName) != config.server_names.end())
            return *it;
    }

    // 3. server_name leading * match
    results.clear();
    std::vector<int> matches; // will store length of the biggest match with each server.

    for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
    {
        const server_config_t config = (*it)->get_config();
        size_t longestMatch = 0; // store the longest match yet for this server.
        // Iterate over each server_name
        for (std::vector<std::string>::const_iterator name_it = config.server_names.begin(); name_it < config.server_names.end(); name_it ++)
        {
            // Check if the server_name starts with a '*', ignore otherwise
            if (name_it->at(0) != '*')
                continue;
            // Keep only the mask part,  ie the non '*' part
            std::string ending = name_it->substr(1, name_it->size() - 1);
            // Check if the hostname ends with the mask
            if (hostName.compare(hostName.length() - ending.length(), ending.length(), ending) == 0)
            {
                // Update longestMatch if this match was bigger than the previous one
                if (ending.size() > longestMatch)
                    longestMatch = ending.size();
            }
        }
        matches.push_back(longestMatch);
    }

    // Keep only the candidates whoose match length's are the maximums of the vector.
    int currentMax = 0;
    for (size_t i = 0; i < candidates.size(); i++)
    {
        if (matches[i] > currentMax)
        {
            currentMax = matches[i];
            results.clear();
            results.push_back(candidates[i]);
        }
        else if (matches[i] == currentMax)
            results.push_back(candidates[i]);
    }

    // If we have a winner returns it, otherwise forget the previous part and start over with the trailing matches.
    if (results.size() == 1)
        return results[0];
    
    // 4. server_name trailing * match
    results.clear();
    matches.clear();
    for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
    {
        const server_config_t config = (*it)->get_config();
        size_t longestMatch = 0;
        for (std::vector<std::string>::const_iterator name_it = config.server_names.begin(); name_it < config.server_names.end(); name_it ++)
        {
            if (name_it->at(name_it->size() - 1) != '*')
                continue;
            std::string starting = name_it->substr(0, name_it->size() - 1);
            if (hostName.compare(0, starting.length(), starting) == 0)
            {
                if (starting.size() > longestMatch)
                    longestMatch = starting.size();
            }
        }
        matches.push_back(longestMatch);
    }
    currentMax = 0;
    for (size_t i = 0; i < candidates.size(); i++)
    {
        if (matches[i] > currentMax)
        {
            currentMax = matches[i];
            results.clear();
            results.push_back(candidates[i]);
        }
        else if (matches[i] == currentMax)
            results.push_back(candidates[i]);
    }
    if (results.size() == 1)
        return results[0];
    
    // If no server is found, fallback to the default server.
    return candidates.front();
}

void				Client::bindCore(WebservCore *core)
{
    _core = core;
}