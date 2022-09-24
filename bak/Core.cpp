#include "Core.hpp"

    WebservCore::WebservCore()
    {

    }

    WebservCore::~WebservCore()
    {

    }

    void WebservCore::setup(std::vector<server_config_t> server_configs)
    {
        // Create the servers and push the server object to _servers[]
        for (std::vector<server_config_t>::iterator config = server_configs.begin(); config != server_configs.end(); config++)
        {
            Server server(*config);
           _servers.push_back(server);
        }

    }

    void WebservCore::run(void)
    {
        // Iterate over all the servers and start listening on all their respective sockets
        for (std::vector<Server>::iterator server = _servers.begin(); server != _servers.end(); server++)
        {
            std::vector<Socket> socks = server->get_listening_sockets();
            for (std::vector<Socket>::iterator sock = socks.begin(); sock < socks.end(); sock++)
            {
                add_op(sock->listen(), POLL_IN);
            }
        }

        int ready;
        OperationBase *op;
        OperationBase *ret;
        while (true)
        {
            ready = poll(_pollfds.data(), _pollfds.size(), 10000);
            std::cout << ready << " out of " <<  _pollfds.size() << std::endl;
            for (int i = 0; i < _pollfds.size(); i++)
            {
                if (_pollfds[i].revents)
                {
                    op = _operations[_pollfds[i].fd];
                    switch (op->type)
                    {
                    case OPERATION_LISTEN:
                        add_op(((ListenOperation *)op)->accept(), POLLIN);
                        break;
                    
                    case OPERATION_READ_REQUEST:
                        ((ReadRequestOperation *)op)->read_request();
                        delete_op(op);
                        break;
                    }
                    break;
                }
            }
        }
    }

    void WebservCore::add_op(OperationBase *op, short events) {
        pollfd pfd;

        pfd.fd = op->fd;
        pfd.events = events;
        _pollfds.push_back(pfd);
        _operations[op->fd] = op;
    }

    void WebservCore::delete_op(OperationBase *op) {
        std::vector<pollfd>::iterator it = _pollfds.begin();
        while (it != _pollfds.end()) {
            if (it->fd == op->fd)
                break;
            it++;
        }
        _operations.erase(op->fd);
        _pollfds.erase(it);
    }