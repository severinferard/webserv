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

#include "Server.hpp"
#include "Socket.hpp"
#include "Response.hpp"
#include "request.hpp"
#include "Client.hpp"

#define OPERATION_BASE      0x01
#define OPERATION_LISTEN    0x02
#define OPERATION_READ_REQ  0x03
#define OPERATION_READ_FILE 0x04

class Request;

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

        Request read_req(void);
        
};

class ReadFileToResponseBody: public OperationBase
{
    private:
        int fd;
        Response *response;
    
    public:
        ReadFileToResponseBody(Server *server, int fd, Response *response);
        ~ReadFileToResponseBody();
        int read();
};


#endif /* OPERATION_HPP */
