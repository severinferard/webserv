#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#define WHITESPACE " \t\r\n\v\f"

std::vector<std::string> splitstr(std::string str, std::string delim);
bool has_whitespace(std::vector<std::string> line);
bool has_whitespace(std::string str);
std::string tolowerstr(std::string str);
std::string trimstr(std::string str);
void print_headers(std::map<std::string, std::string> headers);
std::string joinstr(std::vector<std::string> strs, std::string delim);
bool isDirectory(std::string uri);
bool isValidHttpMethod(std::string method);
bool isValidHttpVersion(std::string version);
std::string toString(const unsigned long& value);
std::string joinPath(const std::string &left, const std::string &right);
bool pathExist(std::string path);
void strVectorToCstrVector(std::vector<std::string> &origin, std::vector<char *> &cstrings);
bool parentDirExists(const std::string &path);
std::string extractRoute(std::string filepath);
std::string extractQueryString(std::string uri);

template<typename K, typename V>
bool hasKey(std::map<K, V>m, K key)
{
    return m.find(key) != m.end();
}

#endif /* UTILS_HPP */