#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <sstream>

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

class UnexpectedTokenException : public ParsingException {
	private:
		std::string token;
		std::string msg;
	public:
	UnexpectedTokenException(std::string token, int line) :  ParsingException(""), token(token) {
		std::ostringstream ss;
		ss << "line " << line << ": " << "Unexpected token: " << "'" << token << "'";
		msg = ss.str();
	};
	~UnexpectedTokenException() throw() {}
	const char* what() const throw()
	{
		return msg.c_str();
	}
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

#endif /* EXCEPTIONS_HPP */
