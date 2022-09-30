#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <map>

#define WHITESPACE " \t\r\n\v\f"

std::vector<std::string> splitstr(std::string str, std::string delim);
bool has_whitespace(std::vector<std::string> line);
bool has_whitespace(std::string str);
std::string tolowerstr(std::string str);
std::string trimstr(std::string str);
void print_headers(std::map<std::string, std::string> headers);
std::string joinstr(std::vector<std::string> strs, std::string delim);

#endif /* UTILS_HPP */
