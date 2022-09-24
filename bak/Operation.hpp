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
#define OPERATION_READ_REQUEST 0x03

class OperationBase
{
    protected:
        Server *_server;
    public:
        int type;
        int fd;

        OperationBase(Server *server, int type, int fd);
        ~OperationBase();
};

class ReadRequestOperation: public OperationBase
{
    private:
       int  _req_fd;
    public:
        ReadRequestOperation(Server *server, int req_fd);
        ~ReadRequestOperation();
        OperationBase *read_request(void);
};

class ListenOperation: public OperationBase
{
    private:
        Socket *_socket;
    public:
        ListenOperation(Server *server, Socket *socket);
        ~ListenOperation();
        OperationBase *accept(void);

    // ReadRequestOperation *accept(void);
};


#endif /* OPERATION_HPP */
