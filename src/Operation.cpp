// #include "Operation.hpp"

// OperationBase::OperationBase(Socket *socket, Server *server, int type, int fd):
//     _socket(socket),
//     _server(server),
//     type(type),
//     fd(fd)
// {
// }

// OperationBase::~OperationBase()
// {
// }

// ListenOperation::ListenOperation(Socket *socket):
//     OperationBase(socket, NULL, OPERATION_LISTEN, socket->get_fd())
// {
    
// }

// ListenOperation::~ListenOperation()
// {
// }

// OperationBase *ListenOperation::accept(void)
// {
//     struct sockaddr_in cli_addr;
//     socklen_t cli_len;
//     int new_fd;

//     cli_len = sizeof(cli_addr);
//     new_fd = ::accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
//     printf("server: got connection from %s port %d\n",
//             inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
//     return new ReadRequestOperation(_socket, new_fd);
// }


// ReadRequestOperation::ReadRequestOperation(Socket *socket, int req_fd):
// OperationBase(socket, NULL, OPERATION_READ_REQ, req_fd)
// {
// }
// ReadRequestOperation::~ReadRequestOperation()
// {

// }

// Request ReadRequestOperation::read_req(void)
// {
//     int ret = recv(fd, _buffer, BUFFER_SIZE, 0);
//     if (ret <= 0)
//     {
//         close(fd);
//         if (ret == 0)
//             throw ConnectionResetByPeerException(_socket, fd);
//         else if (ret == -1)
//             throw std::runtime_error("Error reading request");
//     }
//     else
//     {
//         _buffer[ret] = 0;
//         std::cout << "Recieved request on address " << _socket->get_host() << ":" << _socket->get_port() << " : " << std::endl;
//         std::cout << _buffer << std::endl;
//         // close(fd); // provisoire
//         // return new ReadFileToResponseBody(_server, )
//     }

//     return Request(_socket->get_servers()->at(0), fd, "/lol");
// }


// ReadFileToResponseBody::ReadFileToResponseBody(Server *server, int fd, Response *response):
// OperationBase(NULL, server, OPERATION_READ_FILE, fd)
// {
// }

// ReadFileToResponseBody::~ReadFileToResponseBody()
// {
// }

// int ReadFileToResponseBody::read()
// {
//     return 0;
// }
