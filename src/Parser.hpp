#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <string>
# include <iostream>
# include <fstream>
# include <vector>
# include <sstream>
 #include <iterator>
 #include <cstdlib>

# include "config.hpp"
# include "exceptions.hpp"

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
		void						parse_error_page(std::ifstream & file, std::vector<error_page_t> & error_pages);
		std::vector<std::string>	parse_index(std::ifstream & file);
		uint32_t					parse_client_max_body_size(std::ifstream & file);
		std::vector<http_methods_e> parse_allowed_methods(std::ifstream & file);
		bool						parse_autoindex(std::ifstream & file);
		std::string					parse_client_body_temp_path(std::ifstream & file);
};


std::ostream &			operator<<( std::ostream & o, Parser const & i );

#endif /* ********************************************************** PARSER_H */