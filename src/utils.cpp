#include "utils.hpp"
#include <cstring>

std::vector<std::string> splitstr(std::string str, std::string delim) {
    std::vector<std::string>	splits;
    size_t			start = 0;
    size_t			end;
    size_t			len;

    while (start < str.length()) {
	end = str.find(delim, start);
	len = (end == std::string::npos) ? end : end - start;
	//std::cout << "start = " << start << '\n';
	//std::cout << "end   = " << end << '\n';
	//std::cout << "len   = " << len << '\n';
	//std::cout << '[' << str.substr(start, len) << ']' << '\n';
	splits.push_back(str.substr(start, len));
	if (end == std::string::npos)
	    break;
	start = end + delim.length();
    }

    return (splits);
}

bool has_whitespace(std::vector<std::string> line) {
    for (size_t i = 0; i < line.size(); i++) {
	for (size_t j = 0; j < line[i].size(); j++) {
	    if (isspace(line[i][j]) && line[i][j] != ' ')
		return true;
	}
    }
    return false;
}

bool has_whitespace(std::string str) {
    for (size_t i = 0; i < str.size(); i++)
	if (isspace(str[i]))
	    return true;
    return false;
}

std::string tolowerstr(std::string str) {
    std::string	low = str;

    for (size_t i = 0; i < str.size(); i++)
	low[i] = tolower(low[i]);
    return low;
}

std::string trimstr(std::string str) {
    std::string	trim = str;

    trim.erase(str.find_last_not_of(WHITESPACE) + 1);
    trim.erase(0, str.find_first_not_of(WHITESPACE));
    return trim;
}

void print_headers(std::map<std::string, std::string> headers) {
    std::map<std::string, std::string>::const_iterator	it;

    std::cout << "{" << std::endl;
    for(it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << std::setw(10) << it->first << "\":    ";
        std::cout << '"' << it->second << '"' << std::endl;
    }
    std::cout << "}" << std::endl;
}

std::string joinstr(std::vector<std::string> strs, std::string delim) {
    std::string	join;

    for (size_t i = 0; i < strs.size(); i++) {
	join += strs[i];
	if (i < strs.size() - 1)
	    join += delim;
    }
    return join;
}

bool isValidHttpMethod(std::string method)
{
    return method == "GET" || method == "HEAD" || method ==  "POST" || method ==  "PUT";
}

bool uriIsDirectory(std::string uri)
{
    return uri[uri.size() - 1] == '/';
}

void    registerFd(int epoll_fd, int fd, uint32_t events)
{
    struct epoll_event ev;

    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        throw std::runtime_error("Error registering fd with epoll");
    }
}

void    modifyFd(int epoll_fd, int fd, uint32_t events)
{
    struct epoll_event ev;

    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        throw std::runtime_error("Error registering fd with epoll");
    }
}

std::string toString(const unsigned long& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}