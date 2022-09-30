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

Request::Request(
	std::string m,
	std::string u,
	std::string v,
	std::map<std::string, std::string> h,
	std::string b):
    method(m), uri(u), version(v), headers(h), body(b) {}

Request::Request() {}

std::ostream& operator<<(std::ostream& os, Request const& r) {
    os << "Method:    \"" << r.method << '"' << '\n';
    os << "URI:       \"" << r.uri << '"' << '\n';
    os << "Version:   \"" << r.version << '"' << '\n';
    os << "Headers:   ";
    print_headers(r.headers);
    os << "Body:      \"" << r.body << '"' << '\n';
    return os;
}

static void add_header(std::map<std::string, std::string> &headers, std::string line) {
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

    if (headers.find(name) == headers.end())
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
    //print_headers(headers);

    // get body
    if (i < lines.size() - 1) {
	std::vector<std::string> subvec(lines.begin() + i + 1, lines.end());
	// read at most Content-Length bytes
	body = joinstr(subvec, "\r\n");
	//std::cout << '[' << body << "]\n";
    }

    return Request(lines[0], lines[1], lines[2], headers, body);
}
