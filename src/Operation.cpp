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
    printf("socket port %d\n", _socket->get_fd());
    
    cli_len = sizeof(cli_addr);
    new_fd = ::accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
    printf("server: got connection from %s port %d\n",
            inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    return new ReadRequestOperation(_socket, new_fd);
}



ReadRequestOperation::ReadRequestOperation(Socket *socket, int req_fd):
    OperationBase(socket, NULL, OPERATION_READ_REQUEST, req_fd),
    _to_read(0)
{
}

ReadRequestOperation::~ReadRequestOperation()
{
}

#define READ_REQUEST_SIZE 256
OperationBase *ReadRequestOperation::read_request(void)
{
    printf("read_request\n");

    char buffer[READ_REQUEST_SIZE] = {0};
    size_t n;

    if (!_to_read)
    {
        _to_read = recv(fd ,NULL, READ_REQUEST_SIZE - 1, MSG_PEEK);
        return NULL;
    }
    recv(fd ,buffer, READ_REQUEST_SIZE - 2, 0);
    _buffer.append(buffer);
    if (_to_read != READ_REQUEST_SIZE - 1)
    {
        close(fd);
        std::cout << "Recieved request on address " << _socket->get_host() << ":" << _socket->get_port() << " : " << _buffer << std::endl;
    }
    _to_read = 0;
    return new OperationBase(_socket, _server, -1, 1);
    
}
