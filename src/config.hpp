#ifndef CONFIG_HPP
#define CONFIG_HPP

# include <iostream>
# include <string>
// # include <cstdint>
# include <stdint.h>
# include <vector>
# include <map>


# define DEFAULT_LISTENING_ADDR "0.0.0.0"
# define DEFAULT_LISTENING_PORT 80
# define DEFAULT_CLIENT_MAX_BODY_SIZE 1000000

typedef struct  host_port_s {
    bool        hostIsSet;
    bool        portIsSet;
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
    std::map<int, error_page_t>   error_pages;
    std::vector<std::string> allowed_methods;
    int                        autoindex;
    std::string                 client_body_temp_path;
    int                         client_max_body_size;
    std::string                 cgi_pass;
}                           location_t;

typedef struct              server_config_s {
    std::vector<std::string>    server_names;
    std::vector<host_port_t>    listen_on;
    std::string                 root;
    std::vector<std::string>    index;
    std::vector<location_t>     locations;
    std::map<int, error_page_t>   error_pages;
    int                         client_max_body_size;
    std::vector<std::string> allowed_methods;
    int                         autoindex;
}                           server_config_t;

#endif /* CONFIG_HPP */
