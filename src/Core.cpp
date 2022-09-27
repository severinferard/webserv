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
    void WebservCore::start_listening_sockets(void)
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
                    add_op(sock->listen(), POLLIN);
                    _sockets.push_back(*sock);
                }
                catch(const AddressAlreadyInUseException& e)
                {                    
                    delete sock;
                    existing_socket = find_socket_on_port(addr->port);
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
        OperationBase       *op;
        OperationBase       *new_op;

        start_listening_sockets();

        while (true)
        {
            if (!(ready = epoll_wait(_epoll_fd, &event, 1, EPOLL_TIMEOUT)))
                continue;
            op = find_op_by_fd(event.data.fd);
            switch (op->type)
            {
            case OPERATION_LISTEN:
                add_op(((ListenOperation *)op)->accept(), POLLIN);
                break;
            case OPERATION_READ_REQ:
                ((ReadRequestOperation *)op)->read_req();
            }
        }
    }

    /**
     * @brief Add an operation to the map and register its fd to epoll with the desired events
     * 
     * @param op Pointer on the operation to add (allocated on the heap)
     * @param events events such as POLLIN or POLLOUT
     */
    void WebservCore::add_op(OperationBase *op, uint32_t events) {
        struct epoll_event ev;

        ev.events = events;
        ev.data.fd = op->fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, op->fd, &ev) == -1) {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }
        _operations[op->fd] = op;
    }

    /**
     * @brief Delete an operation from the map. This function does not close the
     * fd linked to the operation and does not remove it from epoll.
     * 
     * @param op 
     */
    void WebservCore::delete_op(OperationBase *op) {
        _operations.erase(op->fd);
    }

    /**
     * @brief Return the operation linked to the desired fd. If not operation is found, NULL is returned.
     * 
     * @param fd 
     * @return OperationBase* or NULL
     */
    OperationBase *WebservCore::find_op_by_fd(int fd)
    {
        try
        {
            return _operations.at(fd);
        }
        catch(const std::out_of_range& e)
        {
            return NULL;
        }
    }

    /**
     * @brief Return the socket already binded to the passed in port. If no socket is found, NULL is returned.
     * 
     * @param port 
     * @return Socket* or NULL
     */
    Socket *WebservCore::find_socket_on_port(uint32_t port)
    {
        for (std::vector<Socket>::iterator sock = _sockets.begin(); sock != _sockets.end(); sock++)
        {
            if (sock->get_port() == port)
                return &(*sock);
        }
        return NULL;
    }