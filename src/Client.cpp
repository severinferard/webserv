#include "Client.hpp"
char Client::_buffer[BUFFER_SIZE];
std::map<int, error_page_t> Client::DEFAULT_ERROR_PAGES;

Client::Client(std::string addr, int port, const Socket *socket, int fd):
    _status(STATUS_WAIT_FOR_REQUEST),
    _server(NULL),
    _location(NULL),
    __log_fd(fd),
    _timedOut(false),
    addr(addr),
    port(port),
    socket(socket),
    connection_fd(fd),
    connectionTimestamp(time(NULL))
{
    _initDefaultErrorPages();
    _request = Request(socket, connection_fd);
    DEBUG("New client connected on endpoint %s:%u from %s:%d", socket->get_host().c_str(), socket->get_port(), addr.c_str(), port);
}

void						Client::_initDefaultErrorPages(void)
{
	error_page_t page;

    page.code = 400;
	page.ret = 400;
	page.path = DEFAULT_ERROR_PAGE_400;
	Client::DEFAULT_ERROR_PAGES[400] = page;

	page.code = 404;
	page.ret = 404;
	page.path = DEFAULT_ERROR_PAGE_404;
	Client::DEFAULT_ERROR_PAGES[404] = page;

	page.code = 405;
	page.ret = 405;
	page.path = DEFAULT_ERROR_PAGE_405;
	Client::DEFAULT_ERROR_PAGES[405] = page;

    
    page.code = 408;
	page.ret = 408;
	page.path = DEFAULT_ERROR_PAGE_408;
	Client::DEFAULT_ERROR_PAGES[408] = page;

	page.code = 411;
	page.ret = 411;
	page.path = DEFAULT_ERROR_PAGE_411;
	Client::DEFAULT_ERROR_PAGES[411] = page;

    page.code = 413;
	page.ret = 413;
	page.path = DEFAULT_ERROR_PAGE_413;
	Client::DEFAULT_ERROR_PAGES[413] = page;

	page.code = 415;
	page.ret = 415;
	page.path = DEFAULT_ERROR_PAGE_415;
	Client::DEFAULT_ERROR_PAGES[415] = page;

	page.code = 500;
	page.ret = 500;
	page.path = DEFAULT_ERROR_PAGE_500;
	Client::DEFAULT_ERROR_PAGES[500] = page;

	page.code = 501;
	page.ret = 501;
	page.path = DEFAULT_ERROR_PAGE_501;
	Client::DEFAULT_ERROR_PAGES[501] = page;

    page.code = 504;
	page.ret = 504;
	page.path = DEFAULT_ERROR_PAGE_504;
	Client::DEFAULT_ERROR_PAGES[504] = page;

    page.code = 505;
	page.ret = 505;
	page.path = DEFAULT_ERROR_PAGE_505;
	Client::DEFAULT_ERROR_PAGES[505] = page;
}

Client::~Client()
{
}

static void filter_filepath(std::string &filepath) {
    size_t  i;

    i = filepath.find('?');
    if (i != std::string::npos)
	filepath = filepath.substr(0, i);

    i = filepath.find('#');
    if (i != std::string::npos)
	filepath = filepath.substr(0, i);
}


std::string getLocationRelativeRoute(location_t location, std::string route)
{
    if (location.modifier == PATH_ENDWITH)
        return route;
    std::string ret = route.substr(location.path.size(), route.size() - location.path.size());
    return ret;
}

void			Client::_handleHead(void)
{
    _handleGet();
}

void			Client::_handleGet(void)
{
    std::string filepath;

    // Generate the file path from the configured root
    if (_location && !_location->root.empty())
    {
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getUri()));
    }
    else
        filepath = joinPath(_server->root, _request.getUri());
    DEBUG("filepath: %s", filepath.c_str());

    if (!pathExist(filepath))
        throw HttpError(HTTP_STATUS_NOT_FOUND); 
    if (isDirectory(filepath))
        {
            _file_fd = -1;
            // Check if the location provides an index that is found in this directory.
            if (_location && !_location->index.empty())
                _file_fd = _findIndex(filepath, _location->index);
            // If no index found on the location level, check if the server provides an index that is found in this directory.
            else if ( !_server->indexes.empty())
                _file_fd = _findIndex(filepath, _server->indexes);
            // If no index can be found, check if directory listing is enabled
            if (_file_fd < 0 && ((_location && _location->autoindex > 0) || (((_location && _location->autoindex < 0) || !_location) && _server->autoindex)) )
            {
                DEBUG("Serving Autoindex");
                _setupAutoIndex(_request.getUri(), filepath);
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
        _core->registerFd(_file_fd, POLLIN, this);
}

void			Client::_handlePost(void) {
    std::string filepath;

    // Generate the file path from the configured folder
    if (_location && _location->client_body_temp_path.size())
        filepath = joinPath(_location->client_body_temp_path, getLocationRelativeRoute(*_location, _request.getUri()));
    else if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getUri()));
    else
        filepath = joinPath(_server->root, _request.getUri());
    filter_filepath(filepath);
    DEBUG("post filepath: %s", filepath.c_str());

    if (!parentDirExists(filepath))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);

    // can't do a POST request on a directory
    if (isDirectory(filepath))
	    throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);

    // POST requests are not 'idempotent' so we append the body to the file
    _file_fd = ::open(filepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);

    _status = STATUS_WAIT_TO_WRITE_FILE;
    _response.setStatus(HTTP_STATUS_CREATED);
    _core->registerFd(_file_fd, POLLIN, this);
}

void			Client::_handlePut(void)
{
   std::string filepath;

    // Generate the file path from the configured folder
    if (_location && _location->client_body_temp_path.size())
        filepath = joinPath(_location->client_body_temp_path, getLocationRelativeRoute(*_location, _request.getUri()));
    else if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getUri()));
    else
        filepath = joinPath(_server->root, _request.getUri());
    filter_filepath(filepath);
    DEBUG("put filepath: %s", filepath.c_str());

    if (!parentDirExists(filepath))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);

    // can't do a POST request on a directory
    if (isDirectory(filepath))
	    throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);

    // POST requests are not 'idempotent' so we append the body to the file
    _file_fd = ::open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

    _status = STATUS_WAIT_TO_WRITE_FILE;
    _response.setStatus(HTTP_STATUS_CREATED);
    _core->registerFd(_file_fd, POLLIN, this);
}

void			Client::_handleDelete(void)
{
    std::string filepath;

    // Generate the file path from the configured root
    if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, _request.getUri());
    else
        filepath = joinPath(_server->root, _request.getUri());
    filter_filepath(filepath);
    DEBUG("filepath: %s", filepath.c_str());

    // can't do a DELETE request on a directory
    if (isDirectory(filepath))
	throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);

    if (!pathExist(filepath.c_str()))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);

    remove(filepath.c_str());

    _status = STATUS_WAIT_TO_WRITE_FILE;
    _response.setStatus(HTTP_STATUS_SUCCESS);
    _core->registerFd(_file_fd, POLLIN, this);
}


void			Client::_handleCgi(void)
{
    int                         stdoutLink[2];
    int                         stdinLink[2];
    int                         status;
    pid_t                       pid;

    std::vector<std::string>    args;
    std::vector<char *>         cArgs;
    std::vector<std::string>    env;
    std::vector<char *>         cEnv;
    std::string                 filepath;

    if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getUri()));
    else
        filepath = joinPath(_server->root, _request.getUri());
    if (!pathExist(filepath))
        throw HttpError(HTTP_STATUS_NOT_FOUND);
    
    if (isDirectory(filepath))
    {
        if (_location->index.size())
        {
            for (std::vector<std::string>::const_iterator it = _location->index.begin(); it != _location->index.end(); it++)
            {
                std::string path = joinPath(filepath, *it);
                if (pathExist(path))
                {
                    filepath = path;
                    break;
                }
            }
        }
    }
    if (!pathExist(filepath))
        throw HttpError(HTTP_STATUS_NOT_FOUND);

    env.push_back("HTTP_METHOD=" + _request.getMethod());
    env.push_back("REQUEST_METHOD=" + _request.getMethod());
    env.push_back("SERVER_PROTOCOL=" + _request.getVersion());
    env.push_back("PATH_INFO=" + _request.getUri());
    env.push_back("SCRIPT_FILENAME=" + filepath);
    env.push_back("REDIRECT_STATUS=CGI");
    args.push_back(_location->cgi_pass);

    strVectorToCstrVector(env, cEnv);
    strVectorToCstrVector(args, cArgs);

    if (pipe(stdoutLink) < 0)
        throw std::runtime_error("Error creating pipe");
    if (pipe(stdinLink) < 0)
        throw std::runtime_error("Error creating pipe");
    
    if ((pid = fork()) == -1)
        throw std::runtime_error("Error while calling fork()");
    
    if (pid == 0)
    {
        dup2(stdoutLink[1], STDOUT_FILENO);
        close(stdoutLink[0]);
        close(stdoutLink[1]);
        close(stdinLink[1]);
        dup2(stdinLink[0], STDIN_FILENO);
        close(stdinLink[0]);
        execve(cArgs[0], &cArgs[0], &cEnv[0]);
        throw std::runtime_error("Execve failed");
    }
    else
    {
        _cgi_pid = pid;
        _cgi_stdin_fd = stdinLink[1];
        _file_fd = stdoutLink[0];
        close(stdoutLink[1]);
        close(stdinLink[0]);
        _core->registerFd(_cgi_stdin_fd, POLLOUT, this);    
        _status = STATUS_WAIT_TO_WRITE_CGI;      
    }
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

void			Client::_onReadyToReadRequest(void)
{
        std::string method;
        std::vector<std::string> allowedMethods;

        char buff[256];
        int ret;

        ret = recv(connection_fd, buff, sizeof(buff), 0);
        if (ret <= 0)
            throw ConnectionResetByPeerException(socket, connection_fd);
        else
        {
            // Append what we just read to the request payload
            _request.appendToPayload(buff, ret);
            // Parse what we already have off the request and return now if we have more to read
            if (!_request.parse())
                return;
        }

        // If we have read the whole request...
        // store the server and location of the request locally for convinience
        _server = _request.getServer();
        _location = _request.getLocation();
    
        if (_location && !_location->cgi_pass.empty())
            return _handleCgi();
        // Handle the request according to its method
        method = _request.getMethod();
        if (method == "GET")
            _handleGet();
        else if (method == "HEAD")
            _handleHead();
        else if (method == "POST")
            _handlePost();
        else if (method == "PUT")
            _handlePut();
        else if (method == "DELETE")
            _handleDelete();
        
}

void			Client::_onReadyToReadFile(void)
{
    bool empty = readFileToResponseBody();
    if (empty)
    {
        _status = STATUS_WAIT_TO_SEND;
        _core->unregisterFd(_file_fd);
        _core->modifyFd(connection_fd, POLLOUT);
    }
}

void			Client::_onReadyToSend(void)
{
    if (_response.getStatus() == 200)
        INFO("%s:%d - %s %s " COLOR_GREEN "%d" COLOR_RESET, addr.c_str(), port, _request.getMethod().c_str(), _request.getUri().c_str(), _response.getStatus());
    else
        INFO("%s:%d - %s %s " COLOR_RED" %d" COLOR_RESET, addr.c_str(), port, _request.getMethod().c_str(), _request.getUri().c_str(), _response.getStatus());
    if (_request.getMethod() == "HEAD")
        _response.setIgnoreBody(true);
    // if (_request.getLocation() && _request.getLocation()->cgi_pass.size())
    //     _response.sendRaw(connection_fd);
    // else
    _response.send(connection_fd);
    DEBUG("Closing");
    close(connection_fd);
    _core->unregisterFd(connection_fd);
}

void            Client::_onReadyToWriteCgi(void)
{
    std::string body;

    body = _request.getBody();
    write(_cgi_stdin_fd, body.c_str(), body.size());
    close(_cgi_stdin_fd);
    _core->unregisterFd(_cgi_stdin_fd);
    _status = STATUS_WAIT_TO_READ_CGI;
    _core->registerFd(_file_fd, POLLIN, this);  
}

void            Client::_onReadyToReadCgi(void)
{
    int status;
    char    buff[1000];
    std::vector<std::string> lines;
    bool    statusSet = false;
    size_t  bodyStart = 0;

    // printf("CGI READY\n");
    int size = read(_file_fd, buff, sizeof(buff) - 1);
    buff[size] = 0;
    // printf("hello %d %s\n", size, buff);
    if (size > 0)
    {
        _cgiPayload.append(buff, size);
    } else if (size < 0)
    {
        throw std::runtime_error("Error reading CGI ouput");
    }
    else
    {
        pid_t finished = waitpid(_cgi_pid, &status, WNOHANG);
        if (finished > 0)
        {
            
            lines = splitstr(_cgiPayload, "\r\n");
            for (size_t i = 0; i < lines.size(); i++)
            {
                if (lines[i].empty())
                    break;
                bodyStart += lines[i].size() + 2;
                size_t col;
                if ((col = lines[i].find(':')) != std::string::npos)
                {
                    std::string fieldName = lines[i].substr(0, col);
                    std::string value = lines[i].substr(col + 1, lines[i].size() - col -1);
                    _response.setHeader(fieldName, value);
                    if (fieldName == "Status")
                    {
                        _response.setStatus(atoi(value.c_str()));
                        statusSet = true;
                    }
                }
            }
            bodyStart += 2;
            _response.appendToBody(_cgiPayload.substr(bodyStart, _cgiPayload.size() - bodyStart));
            if (!statusSet)
                _response.setStatus(HTTP_STATUS_SUCCESS);
            _core->unregisterFd(_file_fd);
            close(_file_fd);
            _status = STATUS_WAIT_TO_SEND;
            _core->modifyFd(connection_fd, POLLOUT);
            return;
        }
        else if (finished < 0)
            throw std::runtime_error("waitpid error");
        // if finished == 0, the cgi hasnt finished yet
    }

}

void			Client::_onReadToWriteFile(void) {
    write(_file_fd, _request.getBody().c_str(), _request.getBody().size());
    _status = STATUS_WAIT_TO_SEND;
    _core->unregisterFd(_file_fd);
    _core->modifyFd(connection_fd, POLLOUT);
}

void			Client::_onHttpError(const HttpError& e)
{
    error_page_t errorPage;
    _response.clearBody();
    ERROR("HTTP Error: %d %s", e.status, Response::HTTP_STATUS[e.status].c_str());
    if (_request.getLocation() && hasKey<int, error_page_t>(_request.getLocation()->error_pages, e.status))
        errorPage = _request.getLocation()->error_pages.at(e.status);
    else if (_request.getServer() && hasKey<int, error_page_t>(_request.getServer()->error_pages, e.status))
        errorPage =  _request.getServer()->error_pages.at(e.status);
    else
    {
        DEBUG("No error page found - serving default");
        errorPage = Client::DEFAULT_ERROR_PAGES[e.status];
    }
    DEBUG("Error page: %s", errorPage.path.c_str());
    _response.setStatus(errorPage.code);
    _file_fd = ::open(errorPage.path.c_str(), O_RDONLY);
    _core->registerFd(_file_fd, POLLIN, this);
    _status = STATUS_WAIT_TO_READ_FILE;
}

void        Client::resume(void)
{
    try
    {
        switch (_status)
        {
        case STATUS_WAIT_FOR_REQUEST:
            _onReadyToReadRequest();
            break;
        
        case STATUS_WAIT_TO_READ_FILE:
            _onReadyToReadFile();
            break;
        
        case STATUS_WAIT_TO_SEND:
            _onReadyToSend();
            break;
        case STATUS_WAIT_TO_READ_DIR:
            _onReadyToReadDir();
            break;

        case STATUS_WAIT_TO_READ_CGI:
            _onReadyToReadCgi();
            break;
        
        case STATUS_WAIT_TO_WRITE_CGI:
            _onReadyToWriteCgi();
            break;

        case STATUS_WAIT_TO_WRITE_FILE:
            _onReadToWriteFile();
            break;
        default:
            break;
        }
    }
    catch(const HttpError& e)
    {
        _onHttpError(e);
    }
    catch (ConnectionResetByPeerException &e)
    {
        std::cout << e.what() << std::endl;
        DEBUG("Closing");
        close(connection_fd);
        _core->unregisterFd(connection_fd);
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
        for (std::vector<host_port_t>::const_iterator listen_it = (*it)->listen_on.begin(); listen_it < (*it)->listen_on.end(); listen_it++)
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
            for (std::vector<host_port_t>::const_iterator listen_it = (*it)->listen_on.begin(); listen_it < (*it)->listen_on.end(); listen_it++)
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
        // Check if we can find the hostname in the server's server_name vector.
        // return directly if an exact match is found as nginx uses the first match.
        if (std::find((*it)->server_names.begin(), (*it)->server_names.end(), hostName) != (*it)->server_names.end())
            return *it;
    }

    // 3. server_name leading * match
    results.clear();
    std::vector<int> matches; // will store length of the biggest match with each server.

    for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
    {
        size_t longestMatch = 0; // store the longest match yet for this server.
        // Iterate over each server_name
        for (std::vector<std::string>::const_iterator name_it = (*it)->server_names.begin(); name_it < (*it)->server_names.end(); name_it ++)
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
        size_t longestMatch = 0;
        for (std::vector<std::string>::const_iterator name_it = (*it)->server_names.begin(); name_it < (*it)->server_names.end(); name_it ++)
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

void				Client::timeout(void)
{
    // Ignore if we're already timedout, and in the process of closing the client to prevent entering an infinite loop.
    if (_timedOut)
        return;

    WARNING("Timeout");
    _timedOut = true;
    if (_status == STATUS_WAIT_FOR_REQUEST)
        return _onHttpError(HttpError(HTTP_STATUS_REQUEST_TIMEOUT));
    return _onHttpError(HttpError(HTTP_STATUS_GATEWAY_TIMEOUT));
    
}