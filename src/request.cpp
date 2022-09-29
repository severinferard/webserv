#include "request.hpp"
#include "utils.hpp"
#include <cstdio>

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

Request::Request() {}

static void add_header(std::map<std::string, std::string> headers, std::string line) {
    std::string	name;
    std::string	value;
    size_t	i;

    i = line.find(':');
    if (i == std::string::npos) {
	std::cout << "400 Bad Request" << '\n';
    }

    name = tolowerstr(line.substr(0, i));
    if (name.find(WHITESPACE) != std::string::npos) {
	std::cout << "400 Bad Request" << '\n';
    }
    //std::cout << '[' << name << ']' << '\n';
    value = trimstr(line.substr(i+1));
    //std::cout << '[' << value << ']' << '\n';

    if (headers.find(name) != headers.end())
	headers[name] = value;
    else
	headers[name] += "," + value;
}

Request parse_request(std::string buf) {
    std::vector<std::string>		lines;
    std::vector<std::string>		req_line;
    std::map<std::string, std::string>	headers;
    size_t				i;
    std::string				body;

    lines = splitstr(buf, "\r\n");
    req_line = splitstr(lines[0], " ");
    if (req_line.size() != 3 || has_whitespace(req_line)) {
	std::cout << "400 Bad Request" << '\n';
	return Request();
    }

    // parse headers until empty line
    for (i = 1; i < lines.size(); i++) {
	if (lines[i].empty())
	    break;
	//std::cout << '[' << lines[i] << ']' << '\n';
	add_header(headers, lines[i]);
    }

    // get body

    return Request();
}
