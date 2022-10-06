#include "Core.hpp"


    WebservCore::WebservCore()
    {
        // _pollfds.reserve(10000000);
    }

    WebservCore::~WebservCore()
    {
    }

    /**
     * @brief Create the virtual 'Server objects and push them to the _servers vector.
     * 
     * @param server_configs vector containing the configuration of each virtual server
     */
    void WebservCore::setup(std::vector<server_config_t> server_configs)
    {
        for (std::vector<server_config_t>::iterator config = server_configs.begin(); config != server_configs.end(); config++)
        {
            Server server(*config);
           _servers.push_back(server);
        }
    }

    /**
     * @brief Goes through the configuration of each virtual server and create the corresponding listening sockets based on the 'listen'
     * directive. The newly created Socket objects are stored in the _sockets vector and ListeningOperation objects are added to the operation queue. 
     */
    void WebservCore::_startListeningSockets(void)
    {
        Socket *sock;
        Socket *existing_socket;

        for (std::vector<Server>::iterator server = _servers.begin(); server != _servers.end(); server++)
        {
            for (std::vector<host_port_t>::const_iterator addr = server->listen_on.begin(); addr != server->listen_on.end(); addr++)
            {
                sock = new Socket(addr->host, addr->port);
                sock->add_server(&(*server));
                try
                {
                    registerFd(sock->listen(1000), POLLIN);
                    _sockets.push_back(*sock);
                }
                catch(const AddressAlreadyInUseException& e)
                {                    
                    delete sock;
                    existing_socket = _findSocketOnPort(addr->port);
                    if (existing_socket)
                        existing_socket->add_server(&(*server));
                    else
                        throw (e);
                }
            }
        }
    }

    void WebservCore::run(void)
    {

        int                 ready;
        Request             request;
        Socket              *sock;
        Client              *client;
        time_t              now;

        _startListeningSockets();

        while (true)
        {
            try {
                now = time(NULL);
                ready = poll(_pollfds.data(), _pollfds.size(), EPOLL_TIMEOUT);
                (void)ready;
                for (size_t i = 0; i < _pollfds.size(); i++)
                {
                    if ((sock = _getListeningSocket(_pollfds[i].fd)))
                    {
                        if (!_pollfds[i].revents)
                            continue;
                        client = sock->acceptConnection();
                        client->bindCore(this);
                        
                        registerFd(client->connection_fd, POLLIN, client);
                    }
                    else
                    {
                        client = _findClient(_pollfds[i].fd);
                        if (difftime(now, client->connectionTimestamp) >= CONNECTION_TIMEOUT_DELAY)
                            client->timeout();
                        if (_pollfds[i].revents)
                            client->resume();
                    }
                }
                
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        for (std::vector<Socket>::iterator it = _sockets.begin(); it < _sockets.end(); it++)
        {
            int t = 1;
            close(it->get_fd());
            setsockopt(it->get_fd(),SOL_SOCKET,SO_REUSEADDR,&t,sizeof(int));
        }
    }

  
    /**
     * @brief Return the socket already binded to the passed in port. If no socket is found, NULL is returned.
     * 
     * @param port 
     * @return Socket* or NULL
     */
    Socket *WebservCore::_findSocketOnPort(uint32_t port)
    {
        for (std::vector<Socket>::iterator sock = _sockets.begin(); sock != _sockets.end(); sock++)
        {
            if (sock->get_port() == port)
                return &(*sock);
        }
        return NULL;
    }

    bool    WebservCore::_isListeningSocket(int fd)
    {
        for (std::vector<Socket>::iterator socket_it = _sockets.begin(); socket_it < _sockets.end(); socket_it++)
        {
            if (socket_it->get_fd() == fd)
                return true;
        }
        return false;
    }


    void    WebservCore::registerFd(int fd, uint32_t events)
    {
        struct pollfd pfd;

        pfd.fd = fd;
        pfd.events = events;
        _pollfds.push_back(pfd);
    }

    void    WebservCore::registerFd(int fd, uint32_t events, Client *client)
    {
        struct pollfd pfd;

        pfd.fd = fd;
        pfd.events = events;
        _pollfds.push_back(pfd);

        _clients[fd] = client;
    }

    void    WebservCore::modifyFd(int fd, uint32_t events)
    {
        for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it < _pollfds.end(); it++)
        {
            if (it->fd == fd)
                it->events = events;
        }
    }

    void    WebservCore::unregisterFd(int fd)
    {
        for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it < _pollfds.end(); it++)
        {
            if (it->fd == fd)
                _pollfds.erase(it);
        }
        _clients.erase(fd);
    }

    Socket *WebservCore:: _getListeningSocket(int fd)
    {
        for (std::vector<Socket>::iterator socket_it = _sockets.begin(); socket_it < _sockets.end(); socket_it++)
        {
            if (socket_it->get_fd() == fd)
                return &(*socket_it);
        }
        return NULL;
    }

    Client * WebservCore:: _findClient(int fd)
    {
        try
        {
            return _clients.at(fd);
        }
        catch(const std::out_of_range& e)
        {
            return NULL;
        }
    }