#include "Client.hpp"
char Client::_buffer[BUFFER_SIZE];

Client::Client(std::string addr, int port, const Socket *socket, int fd):
    _status(STATUS_WAIT_FOR_REQUEST),
    addr(addr),
    port(port),
    socket(socket),
    connection_fd(fd)
{
}

Client::~Client()
{
}

int       openFile(std::string filename)
{
    int ret = open(filename.c_str(), O_RDONLY);
    if (ret >= 0)
        return ret;
    throw std::runtime_error("404");
}

epoll_operation_t makeEpollOperation(int fd, uint32_t events, int op_type)
{
    epoll_operation_t op;

    op.fd = fd;
    op.events = events;
    op.type = op_type;
    return op;
}


void        Client::readRequest(void)
{
    int ret = recv(connection_fd, _buffer, BUFFER_SIZE, 0);
    if (ret <= 0)
    {
        close(connection_fd);
        if (ret == 0)
            throw ConnectionResetByPeerException(socket, connection_fd);
        else if (ret == -1)
            throw std::runtime_error("Error reading request");
    }
    else
    {
        _buffer[ret] = 0;
        _request = Request(connection_fd, _buffer);
        _request.parse();
        std::cout << _request << std::endl;
        Server *server = findServer();
        const location_t *loc = server->findLocation(_request.getUri());
        std::string filepath;
        if (loc)
            std::cout << "location: " << loc->path << std::endl;
        else
            std::cout << "location: No location" << std::endl;

        if (loc && !loc->root.empty())
            filepath = loc->root + _request.getUri();
        else
            filepath = server->get_config().root + _request.getUri();
        std::cout << "filepath: " << filepath << std::endl;
        _file_fd = openFile(filepath);

    }
}

bool	Client::readFileToResponseBody(void)
{
    int ret = read(_file_fd, _buffer, BUFFER_SIZE - 1);
    _buffer[ret] = 0; 
    if (ret > 0)
        _response.appendToBody(_buffer);
    if (ret < BUFFER_SIZE - 1 ||  ret <= 0)
    {
        close(_file_fd);
        return true;
    }
    return false;
}

void        Client::resume(int epoll_fd, std::map<int, Client *> *clients)
{
    epoll_operation_t ret;
    bool            empty;

    try
    {
        switch (_status)
        {
        case STATUS_WAIT_FOR_REQUEST:
            readRequest();
            _status = STATUS_WAIT_TO_READ_FILE;
            registerFd(epoll_fd, _file_fd, EPOLLIN);
            (*clients)[_file_fd] = this;
            break;
        
        case STATUS_WAIT_TO_READ_FILE:
            empty = readFileToResponseBody();
            if (empty)
            {
                _response.setStatus(HTTP_STATUS_200);
                _status = STATUS_WAIT_TO_SEND;
                clients->erase(_file_fd);
                modifyFd(epoll_fd, connection_fd, EPOLLOUT);
            }
            break;
        
        case STATUS_WAIT_TO_SEND:
            printf("ready to send\n");
            _response.send(connection_fd);
            close(connection_fd);
            clients->erase(connection_fd);
            break;

        default:
            break;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        send(connection_fd, e.what(), strlen(e.what()), 0);
        close(connection_fd);
    }
    
}

Server *			Client::findServer(void)
{
    std::map<std::string, std::string>::iterator host_it;
    const std::vector<Server *> *servers = socket->get_servers();

    // If there is only 1 server, return it.
    if (servers->size() == 1)
        return servers->front();

    // Check if the request containers a "Host" header, and return the 1st server if it doesnt.
    host_it = _request.getHeaders().find("Host");
    // if (host_it == _request.getHeaders().end())
    //     return servers->front();
    

    // Check if the "Host" header can be found in one of the servers.
    for (std::vector<Server *>::const_iterator server_it = servers->begin(); server_it < servers->end(); server_it++)
    {
        const server_config_t config = (*server_it)->get_config();

        if (std::find(config.server_names.begin(), config.server_names.end(), host_it->second) != config.server_names.end())
            return *server_it;
        
    }
    // Return the first server as a fallback.
    return servers->front();
}