#include "Client.hpp"

char                        Client::_buffer[BUFFER_SIZE];
std::map<int, error_page_t> Client::DEFAULT_ERROR_PAGES = Client::initDefaultErrorPages();


Client::Client(WebservCore *core, std::string addr, int port, const Socket *socket, int fd):
    _core(core),
    _status(STATUS_WAIT_FOR_REQUEST),
    _request(socket, fd),
    _server(NULL),
    _location(NULL),
    __log_fd(fd),
    _timedOut(false),
    _isClosed(false),
    addr(addr),
    port(port),
    socket(socket),
    connection_fd(fd),
    connectionTimestamp(time(NULL))
{
    // Save te current timestamp to calculate the request time
    gettimeofday(&_t0, NULL);
    // Add a callback on the client socket fd to read the request
    _setCallback(fd, &Client::_onReadyToReadRequest, POLLIN);
    // Create the Request object to handle the request
    
    DEBUG("New client connected on endpoint %s:%u from %s:%d", socket->get_host().c_str(), socket->get_port(), addr.c_str(), port);
}

Client::~Client()
{
}

// Class method called once to initialize the Client::DEFAULT_ERROR_PAGES constant
std::map<int, error_page_t>						Client::initDefaultErrorPages(void)
{
    std::map<int, error_page_t> ret;
	error_page_t page;

    for (size_t i = 0; i < sizeof(HTTP_ERRORS_STR) / sizeof(const char *); i++)
    {
        int code = atoi(HTTP_ERRORS_STR[i]);
        page.code = code;
        page.ret =  code;
        page.path = std::string(DEFAULT_ERROR_PAGES_ROOT) + HTTP_ERRORS_STR[i] + ".html";
        ret[code] =  page;
    }
    
    return ret;
}

// Given a location and a route, return the route relative to that location.
// Example: location: /directory and route /directory/foo, return /foo
std::string getLocationRelativeRoute(location_t location, std::string route)
{
    if (location.modifier == PATH_ENDWITH)
        return route;
    std::string ret = route.substr(location.path.size(), route.size() - location.path.size());
    return ret;
}

void			Client::_handleHead(void)
{
    // HEAD requests are handled exactly like GET, except that we don't send the request body in the Response::send()
    _response.setIgnoreBody(true);
    _handleGet();
}

void			Client::_handleGet(void)
{
    std::string filepath;

    // Generate the file path from the configured root
    if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getRoute()));
    else
        filepath = joinPath(_server->root, _request.getRoute());
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
                _response.setHeader("Content-Type", "text/html");
                _setupAutoIndex(_request.getRoute(), filepath);
                return;
            }
            // Otherwise throw a 404. (Could have been 403 tho, but tester requires 404)
            if (_file_fd < 0)
                throw HttpError(HTTP_STATUS_NOT_FOUND);
        }
        else
        {
            _file_fd = ::open(filepath.c_str(), O_RDONLY);
        }
        
        if (_file_fd <= 0)
            throw HttpError(HTTP_STATUS_NOT_FOUND);
        _setCallback(_file_fd,  &Client::_onReadyToReadFile, POLLIN);
        _response.setStatus(HTTP_STATUS_SUCCESS);
        _response.setHeader("Content-Type", Response::getContentType(_request.getRoute()));
}


void            Client::_handleFormUpload(void)
{
    printf("_handleFormUpload\n");
    uploadedFile_t                      uplaodedFile;
    std::string                         boundary;
    size_t                              pos;
    std::map<std::string, std::string>  headers = _request.getHeaders();
    std::vector<std::string>            chunks;

     _response.setStatus(HTTP_STATUS_SUCCESS);
    if (!hasKey<std::string, std::string>(headers, "content-type") || headers["content-type"].find("multipart/form-data") == std::string::npos)
        return  _setCallback(connection_fd, &Client::_onReadyToSend, POLLOUT);
    pos = headers["content-type"].find("boundary=");
    if (pos == std::string::npos)
        return  _setCallback(connection_fd, &Client::_onReadyToSend, POLLOUT);
    boundary = "--" + headers["content-type"].substr(pos + 9);
    chunks = splitstr(_request.body, boundary);
    for (std::vector<std::string>::iterator chunk = chunks.begin() + 1; chunk != chunks.end(); chunk++)
    {
        std::string contentDisposition;
        std::string filename;
        size_t bodyStart = 2;
        size_t pos;
        size_t semicol;

        if (*chunk == "--\r\n")
            break;
        std::vector<std::string> chunkHeaders = splitstr(*chunk, "\r\n");
        for (std::vector<std::string>::iterator header = chunkHeaders.begin() + 1; header != chunkHeaders.end(); header++)
        {
            bodyStart += header->size() + 2;
            if (header->empty())
                break;
            if ((pos = header->find("Content-Disposition: ")) != std::string::npos)
            {
                if (pos != 0)
                    throw HttpError(HTTP_STATUS_BAD_REQUEST);
                semicol = header->find_first_of(";");
                if (semicol == std::string::npos)
                    throw HttpError(HTTP_STATUS_BAD_REQUEST);
                contentDisposition = header->substr(21, semicol - 21);
                pos = header->find("filename=");
                if (pos == std::string::npos)
                    break;
                filename = header->substr(pos + 10, header->size() - (pos + 10) - 1);
            }
            
        }
        if (contentDisposition.empty() || contentDisposition != "form-data")
            continue;
        if (filename.empty())
            continue;
        printf("OK %s\n", filename.c_str());
        printf("towrite %s\n", chunk->substr(bodyStart, chunk->size() - bodyStart - 2).c_str());
        uplaodedFile.filename = filename;
        uplaodedFile.content = chunk->substr(bodyStart, chunk->size() - bodyStart - 2);
        _uploadedFiles.push_back(uplaodedFile);
    }
    if (_uploadedFiles.size())
        _saveNextFile();
    else
        _setCallback(connection_fd, &Client::_onReadyToSend, POLLOUT);
}

void            Client::_saveNextFile(void)
{
    printf("_saveNextFile\n");
    std::string filepath;

   if (_location && _location->client_body_temp_path.size())
        filepath = joinPath(_location->client_body_temp_path, getLocationRelativeRoute(*_location, _request.getRoute()), _uploadedFiles[0].filename);
    else if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getRoute()),  _uploadedFiles[0].filename);
    else
        filepath = joinPath(_server->root, _request.getRoute(),  _uploadedFiles[0].filename);
    DEBUG("post filepath: %s", filepath.c_str());

    if (!parentDirExists(filepath))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);
    
    _file_fd = ::open(filepath.c_str(), O_WRONLY | O_CREAT, 0644);
    _setCallback(_file_fd,  &Client::_onReadyToWriteUploadedFile, POLLOUT);

}

void			Client::_handlePost(void) {
    std::string filepath;

    // Generate the file path from the configured folder
    if (_location && _location->client_body_temp_path.size())
        filepath = joinPath(_location->client_body_temp_path, getLocationRelativeRoute(*_location, _request.getRoute()));
    else if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getRoute()));
    else
        filepath = joinPath(_server->root, _request.getRoute());
    DEBUG("post filepath: %s", filepath.c_str());

    if (!parentDirExists(filepath))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);

    // Parse form data if POST on a directory
    if (isDirectory(filepath))
    {
        return _handleFormUpload();
    }


    // POST requests are not 'idempotent' so we append the body to the file
    _file_fd = ::open(filepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);

    _response.setStatus(HTTP_STATUS_CREATED);
    _setCallback(_file_fd,  &Client::_onReadyToWriteFile, POLLOUT);
}

void			Client::_handlePut(void)
{
   std::string filepath;

    // Generate the file path from the configured folder
    if (_location && _location->client_body_temp_path.size())
        filepath = joinPath(_location->client_body_temp_path, getLocationRelativeRoute(*_location, _request.getRoute()));
    else if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getRoute()));
    else
        filepath = joinPath(_server->root, _request.getRoute());
    DEBUG("put filepath: %s", filepath.c_str());

    if (!parentDirExists(filepath))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);

    // can't do a POST request on a directory
    if (isDirectory(filepath))
	    throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);

    // POST requests are not 'idempotent' so we append the body to the file
    _file_fd = ::open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

    _setCallback(_file_fd,  &Client::_onReadyToWriteFile, POLLIN);
    _response.setStatus(HTTP_STATUS_CREATED);
}

void			Client::_handleDelete(void)
{
    std::string filepath;

    // Generate the file path from the configured root
    if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, _request.getRoute());
    else
        filepath = joinPath(_server->root, _request.getRoute());
    DEBUG("filepath: %s", filepath.c_str());

    // can't do a DELETE request on a directory
    if (isDirectory(filepath))
	throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);

    if (!pathExist(filepath.c_str()))
	    throw HttpError(HTTP_STATUS_NOT_FOUND);

    remove(filepath.c_str());

    _setCallback(_file_fd,  &Client::_onReadyToWriteFile, POLLIN);
    _response.setStatus(HTTP_STATUS_SUCCESS);
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

    DEBUG("Sending request to CGI");
    if (_location && !_location->root.empty())
        filepath = joinPath(_location->root, getLocationRelativeRoute(*_location, _request.getRoute()));
    else
        filepath = joinPath(_server->root, _request.getRoute());

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
    env.push_back("HTTP_METHOD=" + _request.getMethod());
    env.push_back("REQUEST_METHOD=" + _request.getMethod());
    env.push_back("SERVER_PROTOCOL=" + _request.getVersion());
    env.push_back("PATH_INFO=" + _request.getUri());
    env.push_back("QUERY_STRING=" + _request.getQueryString());
    env.push_back("SCRIPT_FILENAME=" + filepath);
    env.push_back("REDIRECT_STATUS=CGI");
    for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); it++)
    {
        std::string var = "HTTP_" + it->first;
        std::transform(var.begin(), var.end(),var.begin(), ::toupper);
        var += "=" + it->second;
        std::replace(var.begin(), var.end(), '-', '_');
        env.push_back(var);
    }
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
        _setCallback(_cgi_stdin_fd,  &Client::_onReadyToWriteCgi, POLLOUT);
        _setCallback(_file_fd,  &Client::_onReadyToReadCgi, POLLIN);
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
        _response.appendToBody(std::string(_buffer, ret));
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

        char buff[1000000];
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
        _status = STATUS_PROCESSING;
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
        _setCallback(connection_fd,  &Client::_onReadyToSend, POLLOUT);
        _clearCallback(_file_fd);
    }
}

void			Client::_onReadyToSend(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    uint64_t delay =  ((now.tv_sec - _t0.tv_sec) * 1000000 + now.tv_usec - _t0.tv_usec) / 1000;

    if (_response.getStatus() >= 200 && _response.getStatus() < 300)
        INFO("%s:%d - %s %s %s " COLOR_GREEN "%d" COLOR_RESET " %ld ms", addr.c_str(), port, _request.getMethod().c_str(), _request.getUri().c_str(), _request.getUserAgent().c_str(), _response.getStatus(), delay);
    else if (_response.getStatus() == 100)
        INFO("%s:%d - %s %s %s " COLOR_BLUE" %d" COLOR_RESET " %ld ms", addr.c_str(), port, _request.getMethod().c_str(), _request.getUri().c_str(), _request.getUserAgent().c_str(), _response.getStatus(), delay);
    else
        INFO("%s:%d - %s %s %s " COLOR_RED" %d" COLOR_RESET " %ld ms", addr.c_str(), port, _request.getMethod().c_str(), _request.getUri().c_str(), _request.getUserAgent().c_str(), _response.getStatus(), delay);
    // if (_request.getLocation() && _request.getLocation()->cgi_pass.size())
    //     _response.sendRaw(connection_fd);
    // else
    _response.send(connection_fd);
    if (_response.keepAlive())
    {
        _clearCallback(connection_fd);
        _setCallback(connection_fd, &Client::_onReadyToReadRequest, POLLIN);
        _response = Response();
        DEBUG("Keeping connection opened");
    }
    else
    {
        _clearCallback(connection_fd);
        close(connection_fd);
        _isClosed = true;
        DEBUG("Closing connection");
    }    
}

void            Client::_onReadyToWriteCgi(void)
{
    size_t size = _request.body.size() < 30000 ?  _request.body.size() : 30000;

    write(_cgi_stdin_fd, _request.body.c_str(), size);
    _request.body.erase(0, size);
    // printf("%ld\n", _request.body.size());
    if (!_request.body.empty())
        return;
    close(_cgi_stdin_fd);
    _clearCallback(_cgi_stdin_fd);
    DEBUG("Done writing to CGI"); 
}

void            Client::_onReadyToReadCgi(void)
{
    int status;
    char    buff[10000000];
    std::vector<std::string> lines;
    bool    statusSet = false;
    size_t  bodyStart = 0;

    int size = read(_file_fd, buff, sizeof(buff) - 1);
    buff[size] = 0;
    // printf("hello %d %s\n", size, buff);
    if (size > 0)
    {
        _cgiPayload.append(buff, size);
        return ;
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
            _clearCallback(_file_fd);
            close(_file_fd);
            _setCallback(connection_fd,  &Client::_onReadyToSend, POLLOUT);
            return;
        }
        else if (finished < 0)
            throw std::runtime_error("waitpid error");
        // if finished == 0, the cgi hasnt finished yet
    }

}

void			Client::_onReadyToWriteUploadedFile(void) {
    printf("_onReadyToWriteUploadedFile\n");
    write(_file_fd, _uploadedFiles[0].content.c_str(), _uploadedFiles[0].content.size());
    _clearCallback(_file_fd);
    _uploadedFiles.erase(_uploadedFiles.begin());
    if (_uploadedFiles.size())
        _saveNextFile();
    else
    {
        _response.setStatus(HTTP_STATUS_SUCCESS);
        _setCallback(connection_fd,  &Client::_onReadyToSend, POLLOUT);
    }
}

void			Client::_onReadyToWriteFile(void) {
    write(_file_fd, _request.getBody().c_str(), _request.getBody().size());
    _clearCallback(_file_fd);
    _setCallback(connection_fd,  &Client::_onReadyToSend, POLLOUT);
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
    _setCallback(_file_fd,  &Client::_onReadyToReadFile, POLLIN);
    _clearCallback(connection_fd);
}

    void			Client::_setCallback(int fd, callback_t cb)
    {
        _callbacks[fd] = cb;
    }
    void			Client::_setCallback(int fd, callback_t cb, uint32_t events)
    {
        _core->registerFd(fd, events, this);
        _callbacks[fd] = cb;
    }

    void			Client::_clearCallback(int fd)
    {
        _core->unregisterFd(fd);
        _callbacks.erase(fd);
    }

bool        Client::resume(int fd)
{
    try
    {
        (this->*(_callbacks[fd]))();
    }
    catch(const HttpError& e)
    {
        _onHttpError(e);
    }
    catch(const Expect100& e)
    {
        _response.clearBody();
        _response.setStatus(HTTP_STATUS_CONTINUE);
        _response.setHeader("Connection", "keep-alive");
        _clearCallback(connection_fd);
        _setCallback(connection_fd,  &Client::_onReadyToSend, POLLOUT);
    }
    catch (ConnectionResetByPeerException &e)
    {
        std::cout << e.what() << std::endl;
        DEBUG("Closing");
        close(connection_fd);
        _clearCallback(connection_fd);
    }
    return _isClosed;
    
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