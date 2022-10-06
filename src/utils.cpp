#include "utils.hpp"
#include <cstring>
#include <cerrno>
#include <unistd.h>

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

    std::cout << "{" << "\n";
    for(it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << "\t" << std::setw(30) << std::left << it->first + ":";
        std::cout << '"' << it->second << '"' << "\n";
    }
    std::cout << "}" << "\n";
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
    return method == "GET" || method == "HEAD" || method ==  "POST" || method ==  "PUT" || method == "DELETE";
}

bool uriIsDirectory(std::string uri)
{
    struct stat pathStat;

    stat(uri.c_str(), &pathStat);
    return S_ISDIR(pathStat.st_mode);
}

std::string toString(const unsigned long& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

#include <cstdio>

std::string joinPath(const std::string &left, const std::string &right)
{
    size_t end = left.find_last_not_of("/");
    size_t start = right.find_first_not_of("/");

    std::string newLeft = (end == std::string::npos) ? "" : left.substr(0, end + 1);
    std::string newRight = (start == std::string::npos) ? "" : right.substr(start);
    return newLeft + "/" + newRight;
}

bool parentDirExists(const std::string &path) {
    size_t	i;
    std::string	parent_dir;

    i = path.rfind('/');
    if (i != std::string::npos) {
	parent_dir = path.substr(0, i);
	std::cout << "parent_dir: [" << parent_dir << "]\n";
	return (access(parent_dir.c_str(), F_OK) == 0);
    }
    return true;
}
