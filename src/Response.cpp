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
    Response::HTTP_STATUS[400] = "Bad Request";
    Response::HTTP_STATUS[403] = "Forbidden";
    Response::HTTP_STATUS[404] = "Not Found";
    Response::HTTP_STATUS[411] = "Length Required";
    Response::HTTP_STATUS[415] = "Unsupported Media Type";
    Response::HTTP_STATUS[500] = "Internal Server Error";
    Response::HTTP_STATUS[501] = "Not Implemented";
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

int Response::getStatus(void) const
{
    return _status;
}

void        Response::setIgnoreBody(bool b)
{
    _ignoreBody = b;
}

void Response::send(int fd)
{
    setHeader("Content-Type", "text/html");
    setHeader("Content-Length", toString(_body.size()));
    setHeader("Connection", "close");

    _payload += "HTTP/1.1 " + toString(_status) + " " + HTTP_STATUS[_status] + "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); it++)
        _payload += it->first + ": " + it->second + "\r\n";

    _payload += "\r\n";
    if (!_ignoreBody)           // don't send body on HEAD request
        _payload += _body;
    ::send(fd, _payload.c_str(), _payload.size(), 0);
}