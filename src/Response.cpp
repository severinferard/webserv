#include "Response.hpp"

Response::Response()
{
}

Response::~Response()
{
}

void Response::setHeader(std::string fieldName, std::string value)
{
    _headers[fieldName] = value;
}

void Response::appendToBody(std::string str)
{
    _body.append(str);
}

void Response::setStatus(std::string status)
{
    _status = status;
}

void Response::send(int fd)
{
    setHeader("Content-Type", "text/html");
    setHeader("Content-Length", toString(_body.size()));
    setHeader("Connection", "Closed");

    _payload += "HTTP/1.1 " + _status + "\n\r";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); it++)
        _payload += it->first + ": " + it->second + "\n\r";

    _payload += "\n\r";
    _payload += _body;
    ::send(fd, _payload.c_str(), _payload.size(), 0);
}