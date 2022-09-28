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