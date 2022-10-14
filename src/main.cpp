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
#include "Core.hpp"
#include <signal.h>

LogLevel Logger::_verbosity = DebugP;

void my_handler(int s){
           printf("Caught signal %d\n",s);
           exit(1); 

}

int main(int argc, char **argv)
{
    std::vector<server_config_t> server_configs;
    WebservCore wscore;

struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);

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