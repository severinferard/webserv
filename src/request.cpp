# include "Request.hpp"

inline std::ostream &operator<<(std::ostream &os, const http_methods_e &method)
{
    switch (method)
    {
    case GET:
        os << "GET";
        break;
    case HEAD:
        os << "HEAD";
        break;
    case POST:
        os << "POST";
        break;
    case DELETE:
        os << "DELETE";
        break;
    case PUT:
        os << "PUT";
        break;
    };
    return os;
}

Request::Request()
{
}

Request::Request(int fd, std::string payload):
_fd(fd),
_payload(payload)
{
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

void        Request::_addHeader(std::string line)
{
    std::string	name;
    std::string	value;
    size_t	i;

    i = line.find(':');
    if (i == std::string::npos) {
	    std::cout << "400 Bad Request" << '\n';
    }

    name = tolowerstr(line.substr(0, i));
    if (has_whitespace(name)) {
	std::cout << "400 Bad Request" << '\n';
    }
    //std::cout << '[' << name << ']' << '\n';

    value = trimstr(line.substr(i+1));
    //std::cout << '[' << value << ']' << '\n';

    if (_headers.find(name) == _headers.end())
	    _headers[name] = value;
    else
	    _headers[name] += "," + value;
}

#include <iterator>
#define PRINT_STRING_VECTOR(vector) std::copy(vector.begin(), vector.end(), std::ostream_iterator<std::string>(std::cout, " ; ")); std::cout << std::endl;

void        Request::parse(void)
{
    printf("parse\n");
    std::vector<std::string>		lines;
    std::vector<std::string>		req_line;
    std::map<std::string, std::string>	headers;
    size_t				i;
    std::string				body;

    lines = splitstr(_payload, "\r\n");
    req_line = splitstr(lines[0], " ");

    if (req_line.size() != 3 ||
        has_whitespace(req_line) ||
        !isValidHttpMethod(req_line[0]))
    {
        std::cout << "400 Bad Request" << '\n';
        throw std::runtime_error("bad request");
    }
    _method = req_line[0];
    _uri = req_line[1];
    _version = req_line[2];

    // parse headers until empty line
    for (i = 1; i < lines.size(); i++)
    {
        if (lines[i].empty())
            break;
        //std::cout << '[' << lines[i] << ']' << '\n';
        _addHeader(lines[i]);
    }
    //print_headers(headers);

    // get body
    if (i < lines.size() - 1)
    {
	    std::vector<std::string> subvec(lines.begin() + i + 1, lines.end());
	    // read at most Content-Length bytes
	    body = joinstr(subvec, "\r\n");
	    //std::cout << '[' << body << "]\n";
    }
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