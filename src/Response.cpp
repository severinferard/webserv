#include "Response.hpp"

std::map<int, std::string> Response::HTTP_STATUS;
std::map<std::string, std::string> Response::MIME_TYPES;

Response::Response()
{
    _initHttpStatus();
    _initMimeTypes();
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
    Response::HTTP_STATUS[405] = "Method Not Allowed";
    Response::HTTP_STATUS[501] = "Not Implemented";
}

void Response::_initMimeTypes(void) {
    Response::MIME_TYPES["html"] = "text/html";
    Response::MIME_TYPES["css"] = "text/css";
    Response::MIME_TYPES["xml"] = "text/xml";
    Response::MIME_TYPES["txt"] = "text/plain";
}

void Response::setHeader(std::string fieldName, std::string value)
{
    _headers[fieldName] = value;
}

void Response::appendToBody(std::string str)
{
    _body.append(str);
}

void Response::setUri(std::string uri) {
    _uri = uri;
}

void Response::setStatus(int status)
{
    _status = status;
}

std::string  Response::get_content_type(std::string ext) {
    if (Response::MIME_TYPES.find(ext) == Response::MIME_TYPES.end())
	return "html";
    return Response::MIME_TYPES[ext];
}

void Response::send(int fd)
{
    std::string	ext;
    std::string	content_type;

    ext = get_extension(_uri);
    setHeader("Content-Type", get_content_type(ext));
    setHeader("Content-Length", toString(_body.size()));
    setHeader("Connection", "close");

    _payload += "HTTP/1.1 " + toString(_status) + " " + HTTP_STATUS[_status] + "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); it++)
        _payload += it->first + ": " + it->second + "\r\n";

    _payload += "\r\n";
    _payload += _body;
    ::send(fd, _payload.c_str(), _payload.size(), 0);
}
