#ifndef OPERATION_HPP
#define OPERATION_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include "./Server.hpp"
#include "./Socket.hpp"

#define OPERATION_BASE      0x01
#define OPERATION_LISTEN    0x02
#define OPERATION_READ_REQ  0x03

class OperationBase
{
    protected:
        Socket *_socket;
        Server *_server;
    public:
        int type;
        int fd;

        OperationBase(Socket *socket, Server *server, int type, int fd);
        ~OperationBase();
};

class ListenOperation: public OperationBase
{
    public:
        ListenOperation(Socket *socket);
        ~ListenOperation();
        OperationBase *accept(void);
};

#define BUFFER_SIZE 65536
class ReadRequestOperation: public OperationBase
{
    private:
        char _buffer[BUFFER_SIZE];
    
    public:
        ReadRequestOperation(Socket *socket, int req_fd);
        ~ReadRequestOperation();

        OperationBase * read_req(void);
        
};


#endif /* OPERATION_HPP */
