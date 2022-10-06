#include "Parser.hpp"

Parser::Parser()
{
}

Parser::~Parser()
{
}


int get_current_line(std::istream& is)
{
    int lineCount = 1;
    is.clear();     // need to clear error bits otherwise tellg returns -1.
    std::streampos originalPos = is.tellg();
    if (originalPos < 0) 
        return -1;
    is.seekg(0);
    char c;
    while ((is.tellg() < originalPos) && is.get(c))
    {
        if (c == '\n') ++lineCount;
    }
    return lineCount;
}

std::string Parser::get_next_token(std::ifstream & file)
{
	std::string ret;
	char c;

	while (file.get(c))
	{
		if ((c == ' ' || c == '\n')  && ret.length())
			return (ret);
		else if (c == ' ' || c == '\n')
		{}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (ret.length())
			{
				file.seekg(-1, file.cur);
				return (ret);
			}
			else
			{
				ret += c;
				return (ret);
			}
		}
		else
		{
			ret += c;
		}
	}
	return ret;
}

std::string Parser::assert_next_token(std::ifstream & file, std::string value)
{
	std::string token = get_next_token(file);
	if (token != value)
		throw UnexpectedTokenException(token, get_current_line(file));
	return token;
}

std::vector<std::string> Parser::parse_allowed_methods(std::ifstream & file)
{
	std::string token = get_next_token(file);
	std::vector<std::string> ret;

	while (token.size() && token != ";") {
		if (token == "GET")
			ret.push_back("GET");
		else if (token == "HEAD")
			ret.push_back("HEAD");
		else if (token == "POST")
			ret.push_back("POST");
		else if (token == "DELETE")
			ret.push_back("DELETE");
		token = get_next_token(file);
	}
	return ret;
}

std::string Parser::parse_root(std::ifstream & file)
{
	std::string ret = get_next_token(file);
	assert_next_token(file, ";");
	return ret;
}

std::string Parser::parse_cgi(std::ifstream & file)
{
	std::string ret = get_next_token(file);
	assert_next_token(file, ";");
	return ret;
}

host_port_t Parser::parse_listen(std::ifstream & file)
{
	std::string token;

	host_port_t host_port;
	token = get_next_token(file);
	size_t semi_col = token.find(':');

	// If the line contains a full ip + port
	if (semi_col != std::string::npos)
	{
		host_port.hostIsSet = true;
		host_port.portIsSet = true;
		host_port.host = token.substr(0, semi_col);
		host_port.port = atoi(token.substr(semi_col + 1, token.size() - 1).c_str());
		// Verify that the IP address is correct.
		if (std::count(host_port.host.begin(), host_port.host.end(), '.') != 3)
			throw InvalidTokenException(host_port.host, get_current_line(file), "Invalid listening address");
	}
	// Otherwise if the address in only partial
	else
	{
		// We concider that the token is meant to be an IP address if it contains at least 1 "." .
		if (token.find('.') != std::string::npos)
		{
			host_port.hostIsSet = true;
			host_port.portIsSet = false;
			host_port.host = token;
			host_port.port = DEFAULT_LISTENING_PORT;
		}
		else
		{
			host_port.portIsSet = true;
			host_port.hostIsSet = false;
			host_port.port = atoi(token.c_str());
			host_port.host = DEFAULT_LISTENING_ADDR;
		}
	}
	assert_next_token(file, ";");
	return host_port;
}

void Parser::parse_error_page(std::ifstream & file, std::map<int, error_page_t> & error_pages)
{
	std::string token;
	std::vector<std::string> tokens;
	error_page_t error_page;

	token = get_next_token(file);
	while (token.size() && token != ";") {
		tokens.push_back(token);
		token = get_next_token(file);
	}
	error_page.path = tokens.back();
	if (tokens[tokens.size() - 2][0] == '=')
	{
		error_page.ret = atoi(tokens[tokens.size() - 2].erase(0, 1).c_str());
		tokens.erase(tokens.begin() + tokens.size() - 2);
	}
	else
	{
		error_page.ret = 0;
	}
	for (size_t i = 0; i < tokens.size() - 1; i++)
	{	
		error_page.code = atoi(tokens[i].c_str());
		error_pages[error_page.code] = error_page;
	}
}

std::vector<std::string> Parser::parse_server_name(std::ifstream & file)
{
	std::string token = get_next_token(file);
	std::vector<std::string> ret;

	while (token.size() && token != ";") {
		ret.push_back(token);
		token = get_next_token(file);
	}
	return ret;
}

std::vector<std::string>	Parser::parse_index(std::ifstream & file)
{
	std::string token = get_next_token(file);
	std::vector<std::string> ret;

	while (token.size() && token != ";") {
		ret.push_back(token);
		token = get_next_token(file);
	}
	return ret;
}

uint32_t					Parser::parse_client_max_body_size(std::ifstream & file)
{
	std::string token = get_next_token(file);
	uint32_t ret = atoi(token.c_str());
	assert_next_token(file, ";");
	return ret;
}

bool						Parser::parse_autoindex(std::ifstream & file)
{
	std::string token;

	token = get_next_token(file);
	if (token != "on" && token != "off")
		throw UnexpectedTokenException(token, get_current_line(file)); 
	assert_next_token(file, ";");
	return token == "on" ? true : false;
}

std::string					Parser::parse_client_body_temp_path(std::ifstream & file)
{
	std::string token;

	token = get_next_token(file);
	assert_next_token(file, ";");
	return token;
}

location_t					Parser::parse_location(std::ifstream & file)
{
	std::string	token;
	location_t	ret;
	ret.autoindex = -1;
	ret.client_body_temp_path = -1;

	ret.modifier = PATH_NO_MODIFIDER;
	
	token = get_next_token(file);
	if (token == "=") {
		ret.modifier = PATH_STRICT;
	} else if (token == "$") {
		ret.modifier = PATH_ENDWITH;
	}

	if (token == "=" || token == "$") {
		token = get_next_token(file);
		if (token == ";" || token == "{")
			throw UnexpectedTokenException(token, get_current_line(file)); 
	}

	ret.path = token;
	assert_next_token(file, "{");
	token = get_next_token(file);
	while (token.size() && token != "}")
	{
		if (token == "root") {
			ret.root = parse_root(file);
		} else if (token == "cgi_pass") {
			ret.cgi_pass = parse_cgi(file);
		} else if (token == "error_page") {
			parse_error_page(file, ret.error_pages);
		} else if (token == "index") {
			ret.index = parse_index(file);
		} else if (token == "allowed_methods") {
			ret.allowed_methods = parse_allowed_methods(file);
		} else if (token == "autoindex") {
			ret.autoindex = parse_autoindex(file);
		} else if (token == "client_body_temp_path") {
			ret.client_body_temp_path = parse_client_body_temp_path(file);
		} else {
			throw UnknownDirectiveException(token, get_current_line(file));
		}
		token = get_next_token(file);
	}
	return ret;
}

server_config_t 			Parser::parse_server(std::ifstream & file)
{
	std::string token;
	server_config_t ret;
	std::string directive_name;

	ret.client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;

	assert_next_token(file, "{");
	token = get_next_token(file);
	while (token.size() && token != "}")
	{
		directive_name = token;
		if (directive_name == "root") {
			ret.root = parse_root(file);
		} else if (directive_name == "location") {
			ret.locations.push_back(parse_location(file));
		} else if (directive_name == "listen") {
			ret.listen_on.push_back(parse_listen(file));
		} else if (directive_name == "server_name") {
			ret.server_names = parse_server_name(file);
		} else if (directive_name == "error_page") {
			parse_error_page(file, ret.error_pages);
		} else if (directive_name == "index") {
			ret.index = parse_index(file);
		} else if (directive_name == "client_max_body_size") {
			ret.client_max_body_size = parse_client_max_body_size(file);
		} else if (directive_name == "allowed_methods") {
			ret.allowed_methods = parse_allowed_methods(file);
		} else if (directive_name == "autoindex") {
			ret.autoindex = parse_autoindex(file);
		}
		else {
			throw UnknownDirectiveException(directive_name, get_current_line(file));
		}
		
		token = get_next_token(file);
	}
	return ret;
}

std::vector<server_config_t> Parser::parse(std::string const & path)
{
	std::vector<server_config_t> servers;
	server_config_t server;
	std::string token;
	std::string directive_name;

	std::ifstream file(path.c_str());	
	if (file.fail())
		throw FileNotFoundException(path);

	while ((token = get_next_token(file)).length())
	{
		directive_name = token;
		if (directive_name == "server")
		{
			servers.push_back(parse_server(file));
		}
	}
	for (std::vector<server_config_t>::const_iterator i = servers.begin(); i != servers.end(); ++i) {
		print_server(*i);
	}
	return servers;
}

#define PRINT_STRING_VECTOR(vector) std::copy(vector.begin(), vector.end(), std::ostream_iterator<std::string>(std::cout, " ")); std::cout << std::endl;

void Parser::print_server(server_config_t server)
{
	std::cout << "server_names: ";
	PRINT_STRING_VECTOR(server.server_names);

	std::cout << "listen_on: " << std::endl;
	for (std::vector<host_port_t>::const_iterator i = server.listen_on.begin(); i != server.listen_on.end(); ++i) {
		std::cout << "\t" << i->host << ":" << i->port << std::endl;
	}

	std::cout << "root: ";
	std::cout << server.root << std::endl;
	std::cout << "autoindex: " << server.autoindex << std::endl;
	std::cout << "index: ";
	PRINT_STRING_VECTOR(server.index);

	std::cout << "error_pages: " << std::endl;
	for (std::map<int, error_page_t>::const_iterator i = server.error_pages.begin(); i != server.error_pages.end(); ++i) {
		std::cout << "\t" << i->second.code << " " << i->second.ret << " " << i->second.path << std::endl;
	}

	std::cout << "allowed_methods: ";
	for (std::vector<std::string>::const_iterator i = server.allowed_methods.begin(); i != server.allowed_methods.end(); ++i) {
		std::cout << *i << " ";
	}
	std::cout << std::endl;

	for (std::vector<location_t>::const_iterator i = server.locations.begin(); i != server.locations.end(); ++i) {
		std::cout << "location: " << i->path << std::endl;
		std::cout << "\t" << "modifier: " << i->modifier << std::endl;
		std::cout << "\t" << "root: " << i->root << std::endl;
		std::cout << "\t" << "index: ";
		PRINT_STRING_VECTOR(i->index);
		std::cout << "\t" << "autoindex: " << i->autoindex << std::endl;
		std::cout << "\t" << "error pages: " << i->error_pages.size() << std::endl;
		std::cout << "\t" << "cgi_pass: " << i->cgi_pass << std::endl;
	}

	std::cout << std::endl << std::endl;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/


/* ************************************************************************** */

