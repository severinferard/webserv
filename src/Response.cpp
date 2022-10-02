#include "Response.hpp"

std::map<int, std::string> Response::HTTP_STATUS;

Response::Response()
{
    _initHttpStatus();
}

Response::~Response()
{
}

void Response::_initHttpStatus(void)
{
    Response::HTTP_STATUS[200] = "OK";
    Response::HTTP_STATUS[404] = "Not Found";
    Response::HTTP_STATUS[403] = "Forbidden";
}

void Response::setHeader(std::string fieldName, std::string value)
{
    _headers[fieldName] = value;
}

void Response::appendToBody(std::string str)
{
    _body.append(str);
}

void Response::setStatus(int status)
{
    _status = status;
}

void Response::send(int fd)
{
    setHeader("Content-Type", "text/html");
    setHeader("Content-Length", toString(_body.size()));
    setHeader("Connection", "Closed");

    _payload += "HTTP/1.1 " + toString(_status) + " " + HTTP_STATUS[_status] + "\n\r";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); it++)
        _payload += it->first + ": " + it->second + "\n\r";

    _payload += "\n\r";
    _payload += _body;
    ::send(fd, _payload.c_str(), _payload.size(), 0);
}