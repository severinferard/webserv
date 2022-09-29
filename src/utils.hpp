#ifndef UTILS_HPP
#define UTILS_HPP

# include <iostream>
# include <vector>

# define WHITESPACE " \t\r\n\v\f"

std::vector<std::string> splitstr(std::string str, std::string delim);
bool has_whitespace(std::vector<std::string> line);
std::string tolowerstr(std::string str);
std::string trimstr(std::string str);

#endif /* UTILS_HPP */
