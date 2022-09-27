#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <map>
#include "./Parser.hpp"
#include "./Operation.hpp"
#include "Core.hpp"

int main(int argc, char **argv)
{
    std::vector<server_config_t> server_configs;
    WebservCore wscore;

    Parser parser;
    try {
        if (argc != 2)
            throw InvalidArgumentsException();
        server_configs = parser.parse(argv[1]);
        wscore.setup(server_configs);
        wscore.run();
    } catch (std::exception & e) {
        std::cerr << "Webserv: " << e.what() << std::endl;
    }
}