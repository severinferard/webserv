# include "utils.hpp"
# include <cstring>

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
