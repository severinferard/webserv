#ifndef REQUEST_HPP
#define REQUEST_HPP

# include <iostream>
# include <map>

enum http_methods_e {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
};

class Request {
    private:
	std::string			    _method;
	std::string			    _uri;
	std::string			    _version;
	std::map<std::string, std::string>  _headers;
	std::string			    _body;

    public:
	Request(std::string m, std::string u, std::string v, std::string h, std::string b);
	Request();
};

Request parse_request(std::string buf);

#endif /* REQUEST_HPP */
