#include "Operation.hpp"

OperationBase::OperationBase(Socket *socket, Server *server, int type, int fd):
    _socket(socket),
    _server(server),
    type(type),
    fd(fd)
{
}

OperationBase::~OperationBase()
{
}

ListenOperation::ListenOperation(Socket *socket):
    OperationBase(socket, NULL, OPERATION_LISTEN, socket->get_fd())
{
    
}

ListenOperation::~ListenOperation()
{
}

OperationBase *ListenOperation::accept(void)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    int new_fd;

    cli_len = sizeof(cli_addr);
    new_fd = ::accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
    printf("server: got connection from %s port %d\n",
            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    return new ReadRequestOperation(_socket, new_fd);
}


ReadRequestOperation::ReadRequestOperation(Socket *socket, int req_fd):
OperationBase(socket, NULL, OPERATION_READ_REQ, req_fd)
{
}
ReadRequestOperation::~ReadRequestOperation()
{

}

OperationBase * ReadRequestOperation::read_req(void)
{
    int ret = recv(fd, _buffer, BUFFER_SIZE, 0);
    if (ret <= 0)
    {
        if (ret == 0)
            printf("Connection closed by client\n");
        else if (ret == -1)
            printf("Error reading request\n");
        close(fd);
    }
    else
    {
        _buffer[ret] = 0;
        std::cout << "Recieved request on address " << _socket->get_host() << ":" << _socket->get_port() << " : " << std::endl;
        std::cout << _buffer << std::endl;
        close(fd); // provisoire
    }

    return NULL;
}
