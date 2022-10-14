#include "Response.hpp"

std::map<int, std::string> Response::HTTP_STATUS = Response::initHttpStatus();
std::map<std::string, std::string> Response::MIME_TYPES = initMimeTypes();

Response::Response()
{
}

Response::~Response()
{
}

std::map<int, std::string>  Response::initHttpStatus(void)
{
    std::map<int, std::string> ret;

    ret[100] = "Continue";
    ret[200] = "OK";
    ret[201] = "Created";
    ret[301] = "Moved Permanently";
    ret[400] = "Bad Request";
    ret[403] = "Forbidden";
    ret[404] = "Not Found";
    ret[408] = "Request Timeout";
    ret[411] = "Length Required";
    ret[413] = "Payload Too Large";
    ret[415] = "Unsupported Media Type";
    ret[500] = "Internal Server Error";
    ret[501] = "Not Implemented";
    ret[504] = "Gateway Timeout";
    ret[505] = "HTTP Version Not Supported";
    return ret;
}

void Response::setHeader(std::string fieldName, std::string value)
{
    _headers[fieldName] = value;
}

void Response::appendToBody(std::string str)
{
    _body.append(str);
}

void Response::appendToBody(std::string str, size_t size)
{
    _body.append(str, size);
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

std::string  Response::getContentType(std::string ext)
{
    if (Response::MIME_TYPES.find(ext) == Response::MIME_TYPES.end())
 	    return "html";
    return Response::MIME_TYPES[ext];
 }

void Response::send(int fd)
{
    if (!hasKey<std::string, std::string>(_headers, "Content-Type") && !hasKey<std::string, std::string>(_headers, "Content-type"))
        setHeader("Content-Type", "text/html");
    if (_body.size())
        setHeader("Content-Length", toString(_body.size()));
    if (!hasKey<std::string, std::string>(_headers, "Connection"))
        setHeader("Connection", "close");

    _payload += "HTTP/1.1 " + toString(_status) + " " + HTTP_STATUS[_status] + "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); it++)
        _payload += it->first + ": " + it->second + "\r\n";

    _payload += "\r\n";
    if (!_ignoreBody)           // don't send body on HEAD request
        _payload += _body;
    ::send(fd, _payload.c_str(), _payload.size(), 0);
}

void Response::sendRaw(int fd)
{
    ::send(fd, _payload.c_str(), _payload.size(), 0);
}

void Response::appendToRawPayload(std::string str)
{
    _payload.append(str);
}

void Response::clearBody(void)
{
    _body = "";
}

bool Response::keepAlive(void)
{
    return hasKey<std::string, std::string>(_headers, "Connection") && _headers["Connection"] == "keep-alive";
}