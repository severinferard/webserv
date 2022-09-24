#ifndef CONFIG_HPP
#define CONFIG_HPP

# include <iostream>
# include <string>
// # include <cstdint>
# include <stdint.h>
# include <vector>

# include "request.hpp"

# define DEFAULT_CLIENT_MAX_BODY_SIZE 1000000

typedef struct  host_port_s {
    std::string host;
    uint32_t    port;
}               host_port_t;

typedef struct  error_page_s {
    uint32_t    code;
    std::string path;
    uint32_t    ret;
}               error_page_t;

enum path_modifier_e {
    PATH_NO_MODIFIDER,
    PATH_STRICT,
    PATH_ENDWITH
};

typedef struct              location_s {
    std::string                 path;
    path_modifier_e             modifier;
    std::string                 root;
    std::vector<std::string>    index;
    std::vector<error_page_t>   error_pages;
    std::vector<http_methods_e> allowed_methods;
    bool                        autoindex;
    std::string                 client_body_temp_path;
}                           location_t;

typedef struct              server_config_s {
    std::vector<std::string>    server_names;
    std::vector<host_port_t>    listen_on;
    std::string                 root;
    std::vector<std::string>    index;
    std::vector<location_t>     locations;
    std::vector<error_page_t>   error_pages;
    uint32_t                    client_max_body_size;
    std::vector<http_methods_e> allowed_methods;
    bool                        autoindex;
}                           server_config_t;

#endif /* CONFIG_HPP */
