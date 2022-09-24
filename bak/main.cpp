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


// ListenOperation *start_server(server_config_t config)
// {
//     int sockfd;
//     struct in_addr host;
//     struct pollfd pfd;
//     struct sockaddr_in serv_addr;

//     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) 
//         throw std::runtime_error("Error opening socket");

//     inet_pton(AF_INET, config.listen_on[0].host.c_str(), &host);
//     memset(&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr = host;
//     serv_addr.sin_port = htons(config.listen_on[0].port);

//     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
//         throw std::runtime_error("ERROR on binding");
//     listen(sockfd,5);
//     return new ListenOperation(sockfd);
// }

// typedef struct s_state {
//     std::vector<pollfd>             pollfds;
//     std::map<int, OperationBase *>  operations;
    
//     void addOp(OperationBase *op, short events) {
//         pollfd pfd;

//         pfd.fd = op->fd;
//         pfd.events = events;
//         pollfds.push_back(pfd);
//         operations[op->fd] = op;
//     }

//     void deleteOp(OperationBase *op) {
//         std::vector<pollfd>::iterator it = pollfds.begin();
//         while (it != pollfds.end()) {
//             if (it->fd == op->fd)
//                 break;
//             it++;
//         }
//         operations.erase(op->fd);
//         pollfds.erase(it);
//     }
// }              t_state;

// void run(std::vector<server_config_t> server_configs)
// {
//     t_state state;
//     OperationBase *op;
//     int ready;

//     for (std::vector<server_config_t>::iterator server = server_configs.begin(); server != server_configs.end(); server++)
//     {
//         state.addOp(start_server(*server), POLLIN);
//     }

//     while (true)
//     {
//         ready = poll(state.pollfds.data(), state.pollfds.size(), 10000);
//         std::cout << ready << " out of " <<  state.pollfds.size() << std::endl;
//         for (int i = 0; i < state.pollfds.size(); i++)
//         {
//             if (state.pollfds[i].revents & POLLIN)
//             {
//                 op = state.operations[state.pollfds[i].fd];
//                 switch (op->type)
//                 {
//                 case OPERATION_LISTEN:
//                     state.addOp(((ListenOperation *)op)->accept(), POLLIN);
//                     state.pollfds[i].revents = 0;
//                     break;
//                 case OPERATION_READ_REQUEST:
//                     ((ReadRequestOperation *)op)->readRequest();
//                     state.deleteOp(op); i--;
//                     break;
//                 }
//                 break;
//             }
//         }
//     }
// }

void test(void)
{
//     std::vector<pollfd> pollfds;
//     std::map<int, OperationBase *> operations;
    

//     int sockfd, newsockfd;
//     struct in_addr host;
//     struct sockaddr_in serv_addr, cli_addr;
//     socklen_t clilen;
//     struct pollfd pfd;

//     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) 
//         throw std::runtime_error("Error opening socket");

//     inet_pton(AF_INET, "0.0.0.0", &host);

//     memset(&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr = host;
//     serv_addr.sin_port = htons(8080);

//     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
//         throw std::runtime_error("ERROR on binding");
//     listen(sockfd,5);


//     pfd.fd = sockfd;
//     pfd.events = POLLIN;
//     pollfds.push_back(pfd);
//     ListenOperation *listenOp = new ListenOperation(sockfd);
//     operations[sockfd] = listenOp;

//     int ready = poll(pollfds.data(), pollfds.size(), 10000);
//     std::cout << ready << std::endl;

//     ReadRequestOperation readReqOp = listenOp->accept();

//     pfd.fd = readReqOp.fd;
//     pfd.events = POLLIN;
//     pollfds.push_back(pfd);
//     ready = poll(pollfds.data(), pollfds.size(), 10000);
//     std::cout << ready << std::endl;
// //     clilen = sizeof(cli_addr);
// // std::cout << "before accept" << std::endl;
// //     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
// //     if (newsockfd < 0) 
// //         throw std::runtime_error("ERROR on accept");

//     // printf("server: got connection from %s port %d\n",
//     //         inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

//     char buffer[256] = {0};
//     int n = read(readReqOp.fd,buffer,255);

//     if (n < 0) throw std::runtime_error("ERROR reading from socket");
//     printf("Here is the message: %s\n",buffer);

//     close(newsockfd);
//     close(sockfd);

}

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
        // start_server();
        // run(server_configs);
    } catch (std::exception & e) {
        std::cerr << "Webserv: " << e.what() << std::endl;
    }
}