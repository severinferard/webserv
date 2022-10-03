#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <string>
# include <iostream>
# include <fstream>
# include <vector>
# include <sstream>
# include <iterator>
# include <cstdlib>
# include <algorithm>
# include "config.hpp"

class ParsingException : public std::runtime_error {
	private:
		std::string token;
		std::string str;
	public:
	ParsingException(std::string s) : runtime_error(s), str(s) {};
	~ParsingException() throw () {}
	virtual const char* what() const throw()
	{
		return str.c_str();
	}
};

class InvalidTokenException : public ParsingException {
	private:
		std::string msg;
	public:
	InvalidTokenException(std::string token, int line, std::string str) :  ParsingException("") {
		std::ostringstream ss;
		ss << "line " << line << ": " << str << ": " << "'" << token << "'";
		msg = ss.str();
	};
	~InvalidTokenException() throw() {}
	const char* what() const throw()
	{
		return msg.c_str();
	}
};

class UnexpectedTokenException : public InvalidTokenException {
	private:
		std::string msg;
	public:
	UnexpectedTokenException(std::string token, int line) :  InvalidTokenException(token, line, "Unexpected token") {};
	~UnexpectedTokenException() throw() {}
};


class UnknownDirectiveException : public ParsingException {
	private:
		std::string token;
		std::string msg;
	public:
	UnknownDirectiveException(std::string token, int line) : ParsingException(""), token(token) {
		std::ostringstream ss;
		ss << "line " << line << ": " << "Unknown directive: " << "'" << token << "'";
		msg = ss.str();
	};
	~UnknownDirectiveException() throw () {}
	const char* what() const throw()
	{
		return msg.c_str();
	}
};

class FileNotFoundException : public std::runtime_error {
	private:
		std::string path;
		std::string msg;
	public:
	FileNotFoundException(std::string path) : runtime_error(""), path(path) {
		std::ostringstream ss;
		ss << path << ": File not found.";
		msg = ss.str();
	};
	~FileNotFoundException() throw() {}
	const char* what() const throw()
	{
		return msg.c_str();
	}
};

class InvalidArgumentsException : public std::runtime_error {
	public:
	InvalidArgumentsException() :runtime_error("Invalid number of arguments.\nUsage: ./webserv config_file") {
	};
	~InvalidArgumentsException() throw() {}
};

class AddressAlreadyInUseException : public std::runtime_error {
	private:
		std::string msg;
	public:
	AddressAlreadyInUseException(uint32_t port) :runtime_error("") {
		std::ostringstream ss;
		ss << "Address already in use: port " << port;
		msg = ss.str();
	};
	const char* what() const throw()
	{
		return msg.c_str();
	}
	~AddressAlreadyInUseException() throw() {}
};

class Parser
{

	public:

		Parser();
		~Parser();
		std::vector<server_config_t> parse(std::string const & path);

	private:
		std::string					get_next_token(std::ifstream & file);
		server_config_t				parse_server(std::ifstream & file);
		std::string					assert_next_token(std::ifstream & file, std::string value);
		void						print_server(server_config_t server);

		server_config_t				parse_server(server_config_t & server);
		location_t					parse_location(std::ifstream & file);
		std::string					parse_root(std::ifstream & file);
		host_port_t					parse_listen(std::ifstream & file);
		std::vector<std::string>	parse_server_name(std::ifstream & file);
		void						parse_error_page(std::ifstream & file, std::map<int, error_page_t> & error_pages);
		void						init_error_pages(std::map<int, error_page_t> & error_pages);
		std::vector<std::string>	parse_index(std::ifstream & file);
		uint32_t					parse_client_max_body_size(std::ifstream & file);
		std::vector<std::string> parse_allowed_methods(std::ifstream & file);
		bool						parse_autoindex(std::ifstream & file);
		std::string					parse_client_body_temp_path(std::ifstream & file);
		std::string					parse_cgi_pass(std::ifstream & file);
};


std::ostream &			operator<<( std::ostream & o, Parser const & i );

#endif /* ********************************************************** PARSER_H */