#include "Core.hpp"


    WebservCore::WebservCore()
    {
        _epoll_fd = epoll_create(1);
        if (_epoll_fd < 0)
            throw std::runtime_error("Failed creating epoll fd");
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
            server_config_t server_config = server->get_config();
            for (std::vector<host_port_t>::iterator addr = server_config.listen_on.begin(); addr != server_config.listen_on.end(); addr++)
            {
                sock = new Socket(addr->host, addr->port);
                sock->add_server(&(*server));
                try
                {
                    registerFd(sock->listen(), POLLIN);
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
        struct epoll_event  event;

        int                 ready;
        Request             request;
        Socket              *sock;
        Client              *client;

        _startListeningSockets();

        while (true)
        {
            try {
                if (!(ready = epoll_wait(_epoll_fd, &event, 1, EPOLL_TIMEOUT)))
                    continue;
                if ((sock = _getListeningSocket(event.data.fd)))
                {
                    client = sock->acceptConnection();
                    client->bindCore(this);
                    std::cout << "New client connected on endpoint " << client->socket->get_host().c_str() << ":" << client->socket->get_port() << " from " << client->addr << ":" << client->port << std::endl;
                    registerFd(client->connection_fd, POLLIN, client);
                }
                else
                {
                    client = _findClient(event.data.fd);
                    client->resume();
                }
                
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
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
        struct epoll_event ev;

        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            throw std::runtime_error("Error registering fd with epoll");
        }
    }

    void    WebservCore::registerFd(int fd, uint32_t events, Client *client)
    {
        struct epoll_event ev;

        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            printf("%s\n", strerror(errno));
            throw std::runtime_error("Error registering fd with epoll");
        }
        _clients[fd] = client;
    }

    void    WebservCore::modifyFd(int fd, uint32_t events)
    {
        struct epoll_event ev;

        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
        {
            throw std::runtime_error("Error registering fd with epoll");
        }
    }

        void    WebservCore::unregisterFd(int fd)
    {
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