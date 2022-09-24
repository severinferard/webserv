#include "Operation.hpp"

OperationBase::OperationBase(Server *server, int type, int fd):
    _server(server),
    type(type),
    fd(fd)
{
}

OperationBase::~OperationBase()
{
}

ListenOperation::ListenOperation(Server *server, Socket *socket):
    OperationBase(server, OPERATION_LISTEN, socket->get_fd()),
    _socket(socket)
{
}

ListenOperation::~ListenOperation()
{
}

// ReadRequestOperation *ListenOperation::accept()
// {
    
// }

OperationBase *ListenOperation::accept(void)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    int new_fd;

    cli_len = sizeof(cli_addr);
    new_fd = ::accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
    printf("server: got connection from %s port %d\n",
            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    return new ReadRequestOperation(_server, new_fd);
}



ReadRequestOperation::ReadRequestOperation(Server *server, int req_fd):
    OperationBase(server, OPERATION_READ_REQUEST, req_fd),
    _req_fd(req_fd)
{
}

ReadRequestOperation::~ReadRequestOperation()
{
}

OperationBase *ReadRequestOperation::read_request(void)
{
    char buffer[256] = {0};
    int n = ::read(fd, buffer, 255);

    if (n < 0) throw std::runtime_error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);
    close(fd);
    return NULL;
}
