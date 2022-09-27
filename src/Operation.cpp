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
    // return new bakReadRequestOperation(_socket, new_fd);
    return new ReadRequestOperation(_socket, new_fd);
}



bakReadRequestOperation::bakReadRequestOperation(Socket *socket, int req_fd):
    OperationBase(socket, NULL, OPERATION_READ_REQUEST, req_fd),
    _to_read(0)
{
}

bakReadRequestOperation::~bakReadRequestOperation()
{
}

#define READ_REQUEST_SIZE 256
OperationBase *bakReadRequestOperation::read_request(void)
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

ReadRequestOperation::ReadRequestOperation(Socket *socket, int req_fd):
OperationBase(socket, NULL, OPERATION_READ_REQ, req_fd),
_next_line_len(0)
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

// OperationBase * ReadRequestOperation::read_req(void)
// {
//     int size;
//     if (!_next_line_len)
//     {
//         size = recv(fd ,_buffer, LINE_BUFFER_SIZE, MSG_PEEK);
//         for (int i = 0; i < size - 1; i++)
//         {
//             if (_buffer[i] == '\r' && _buffer[i + 1] == '\n')
//             {
//                 _next_line_len = i + 2;
//                 return NULL;
//             }
//         }
//         _next_line_len = LINE_BUFFER_SIZE;
//         return NULL;
//     }
//     else
//     {
//         printf("_next_line_len %d\n", _next_line_len);
//         size = recv(fd ,_buffer, _next_line_len, 0);
//         printf("size %d\n", size);
//         // _buffer[size] = 0;
//         if (_buffer[size - 2] != '\r' || _buffer[size - 1] != '\n')
//         {
//             printf("bad req\n");
//         }
//         _buffer[size - 2] = 0;
//         _buffer[size - 1] = 0;
//         _buffer[size] = 0;
//         printf("%s\n", _buffer);
//         _next_line_len = 0;
//         return NULL;
//     }
// }
