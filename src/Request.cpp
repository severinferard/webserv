# include "Request.hpp"

static const std::string LINE_DELIMITER = "\r\n";

Request::Request():
_server(NULL),
_location(NULL),
_headerReceived(false)
{
}

Request::Request(const Socket *sock, int connection_fd):
_fd(sock->get_fd()),
__log_fd(connection_fd),
_socket(sock),
_server(NULL),
_location(NULL),
_headerReceived(false)
{
    // LOG(DebugP, "new request %d", 42);
}

Request::~Request()
{
}

std::string                         Request::getUri(void) const
{
    return _uri;
}

std::string                         Request::getMethod(void) const
{
    return _method;
}

std::string                         Request::getVersion(void) const
{
    return _version;
}

const std::map<std::string, std::string>  &Request::getHeaders(void) const
{
    return _headers;
}

std::string                         Request::getBody(void) const
{
    return _body;
}

int         Request::getFd(void) const
{
    return _fd;
}

std::string Request::getPayload(void) const
{
    return _payload;
}

location_t                          *Request::getLocation(void) const
{
    return _location;
}

Server                              *Request::getServer(void) const
{
    return _server;
}

void        Request::_setHeaders(std::map<std::string, std::string> headers)
{
    _headers = headers;
}

void        Request::_addHeader(std::string line, std::map<std::string, std::string> &headers)
{
    std::string	name;
    std::string	value;
    size_t	i;

    i = line.find(':');
    if (i == std::string::npos)
	    throw HttpError(HTTP_STATUS_BAD_REQUEST);

    name = tolowerstr(line.substr(0, i));
    if (has_whitespace(name))
	    throw HttpError(HTTP_STATUS_BAD_REQUEST);

    value = trimstr(line.substr(i+1));

    if (headers.find(name) == headers.end())
	    headers[name] = value;
    else
	    headers[name] += "," + value;
}

std::vector<std::string> splitLines(std::string payload)
{
    size_t pos;
    std::vector<std::string> ret;

    while (payload.size())
    {
        pos = payload.find(LINE_DELIMITER, 0);
        if (pos != std::string::npos)
        {
            ret.push_back(payload.substr(0, pos + LINE_DELIMITER.size()));
            payload.erase(payload.begin(), payload.begin() + pos + LINE_DELIMITER.size());
        }
        else
        {
            ret.push_back(payload);
            payload.erase(payload.begin(), payload.end());
        }
    }
    return ret;
}

static bool isLineComplete(std::string line)
{
    if (line.size() < 2)
        return false;
    return line.substr(line.size() - 2, 2) == "\r\n";
}

static bool isEmptyLine(std::string line)
{
    return line == "\r\n";
}

void        Request::validate(std::vector<std::string>lines, size_t headerLineCount)
{
    std::vector<std::string>            allowedMethods;

    // Throw 400 if the request doesnt provide a Host header
    if (!hasKey<std::string, std::string>(_headers, "Host") && !hasKey<std::string, std::string>(_headers, "host"))
        throw HttpError(HTTP_STATUS_BAD_REQUEST);

    // Find the right server and location from the headers data
    _server = findServer();
    DEBUG("root: %s", _server->root.c_str());

    _location = const_cast<location_t *>(_server->findLocation(_uri));
    DEBUG("location: %s", (_location ? _location->path.c_str() : "No location"));

    // Check if the methods are restricted for this route
    allowedMethods = _location && !_location->allowed_methods.empty()
            ? _location->allowed_methods
            : _server->allowed_methods;

    // If so, return 405 if the method is not allowed
    if (!allowedMethods.empty() && std::find(allowedMethods.begin(), allowedMethods.end(), _method) == allowedMethods.end())
        throw HttpError(HTTP_STATUS_METHOD_NOT_ALLOWED);
    
    // Calculate the position of the first character of the body in the payload
    _bodyStart = 0;
    for (size_t i = 0; i < headerLineCount; i++)
        _bodyStart += lines[i].size();
    _bodyStart += LINE_DELIMITER.size(); // add empty line

    if (_method == "POST")
    {
        std::string transferEncoding;
        if (hasKey<std::string, std::string>(_headers, "Transfer-Encoding"))
        {
            transferEncoding = _headers.at("Transfer-Encoding");
            if (transferEncoding != "chunked")
                throw HttpError(HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE);
        }

        if (hasKey<std::string, std::string>(_headers, "content-length"))
            _headers["Content-Length"] = _headers["content-length"];
        // Return 411 if the Content-Length header is not set
        if (transferEncoding.empty() && !hasKey<std::string, std::string>(_headers, "Content-Length"))
            throw HttpError(HTTP_STATUS_LENGTH_REQUIRED);

        // Read the Content-Length and check if it's valid
        char *end;
        _contentLength = strtoul(_headers["Content-Length"].c_str(), &end, 10);
        if (end == _headers["Content-Length"].c_str() || _contentLength <= 0)
            throw HttpError(HTTP_STATUS_BAD_REQUEST);
    }
}

int        Request::parse(void)
{
    std::vector<std::string>            lines;
    std::vector<std::string>            requestLine;
    std::map<std::string, std::string>  headers;

    if (!_headerReceived)
    {
        // Spit the buffer into lines arround the /r/n
        lines = splitLines(_payload);

        // Retrun now if we don't have the first line completed
        if (!isLineComplete(lines[0]))
            return false;

        // Parse the first line with method, uri and protocol version and check if it's correct
        requestLine = splitstr(lines[0].substr(0, lines[0].size() - 2), " ");
        if (requestLine.size() != 3 || !isValidHttpMethod(requestLine[0]))
            throw HttpError(HTTP_STATUS_BAD_REQUEST);
        
        _method = requestLine[0];
        _uri = requestLine[1];
        _version = requestLine[2];

        if (_method.empty())
            throw HttpError(HTTP_STATUS_BAD_REQUEST);
 	    if (!isValidHttpMethod(_method))
            throw HttpError(HTTP_STATUS_NOT_IMPLEMENTED);
        
        if (!isValidHttpVersion(_version))
            throw HttpError(HTTP_STATUS_VERSION_NOT_SUPPORTED);

        
        // Parse headers until empty line or a line not completed
        size_t headerLineCount = 1;
        printf("%d %d %s\n", isEmptyLine(lines[headerLineCount]), isLineComplete(lines[headerLineCount]), lines[headerLineCount].c_str());
        while (headerLineCount < lines.size() && !isEmptyLine(lines[headerLineCount]) && isLineComplete(lines[headerLineCount]))
        {
            _addHeader(lines[headerLineCount], headers);
            headerLineCount++;
        }

        printf("headerLineCount %ld real %ld\n", headerLineCount, lines.size());

        // Check if we reached the end of the header
        if (isEmptyLine(lines[headerLineCount]))
        {   
            _headerReceived = true;
            _setHeaders(headers);

            validate(lines, headerLineCount);
        }
        else
            return false;
    }

    // This part is executed only when the header has already been received
    if (_method == "GET")
    {
        return true;
    }
    else if (_method == "POST")
    {
        if (_payload.size() - _bodyStart < _contentLength)
            return false;
        _body = _payload.substr(_bodyStart, _payload.size() - _bodyStart);
        printf("BODY %s\n", _body.c_str());
    }
    return true;
}

 void                                Request::appendToPayload(char *str, size_t size)
 {
    _payload.append(str, size);
 }


std::ostream& operator<<(std::ostream& os, Request const& r) {
    os << std::setw(10) << "Method:    \"" << r.getMethod() << '"' << "\n";
    os << std::setw(10) << "URI:       \"" << r.getUri() << '"' << "\n";
    os << std::setw(10) << "Version:   \"" << r.getVersion() << '"' << "\n";
    os << std::setw(10) << "Headers:   " << "\n";
    print_headers(r.getHeaders());
    os << std::setw(10) << "Body:      \"" << r.getBody() << '"' << "\n";
    return os;
}

Server *			Request::findServer(void)
{
    std::vector<Server *> candidates = *_socket->get_servers();
    std::vector<Server *> results;

    // If there is only 1 server, return it.
    if (candidates.size() == 1)
        return candidates.front();

    // 1. Listen specificity - check if both the IP and port are set
    for (std::vector<Server *>::const_iterator it = candidates.begin(); it < candidates.end(); it++)
    {
        for (std::vector<host_port_t>::const_iterator listen_it = (*it)->listen_on.begin(); listen_it < (*it)->listen_on.end(); listen_it++)
        {
            // Check if this "listen" directive fits the current client _socket and if so check wether both IP and port are set.
            if (listen_it->host == _socket->get_host() && listen_it->port == _socket->get_port() && (listen_it->hostIsSet && listen_it->portIsSet))
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
                // Check if this "listen" directive fits the current client _socket and if so check wether both IP and port are set.
                if (listen_it->host == _socket->get_host() && listen_it->port == _socket->get_port() && listen_it->hostIsSet)
                    results.push_back(*it);
            }
        }
    }

    if (results.size() == 1)
        return results[0];
    if (results.size() > 1)
        candidates = results;


    std::map<std::string, std::string>::const_iterator host_it = _headers.find("host");
    if (host_it == _headers.end())
        host_it = _headers.find("Host");
    // If the request doesnt contain a Host header, return the first candidate.
    if (host_it == _headers.end())
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
