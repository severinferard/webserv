#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <string>
# include <algorithm>
# include "Socket.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "epoll.h"
# include <fcntl.h>
# include "utils.hpp"
#include <sstream>

typedef enum ClientStatus_s {
	STATUS_WAIT_FOR_REQUEST,
	STATUS_WAIT_TO_READ_FILE,
	STATUS_WAIT_TO_SEND,
}           ClientStatus_t;

#define BUFFER_SIZE 65139

class Socket;
class Request;
class WebservCore;

#define EPOLL_OP_NOOP -1
#define EPOLL_OP_ADD 1
#define EPOLL_OP_MODIFY 2
#define EPOLL_OP_REMOVE 3

class HttpError : public std::runtime_error {
	public:
		HttpError(): std::runtime_error("Http Error"){};
		HttpError(std::string msg): std::runtime_error(msg){};
		~HttpError() throw() {}
};

class HttpError401 : public HttpError {
	public:
		HttpError401(): HttpError("Http Error: 401 Length Required"){};
		~HttpError401() throw() {}
};

class HttpError403 : public HttpError {
	public:
		HttpError403(): HttpError("Http Error: 403 Forbidden"){};
		~HttpError403() throw() {}
};

class HttpError404 : public HttpError {
	public:
		HttpError404(): HttpError("Http Error: 404 Not Found"){};
		~HttpError404() throw() {}
};

class HttpError405 : public HttpError {
	public:
		HttpError405(): HttpError("Http Error: 405 Method not allowed"){};
		~HttpError405() throw() {}
};

class Client
{
	private:
		static char		_buffer[BUFFER_SIZE];
		ClientStatus_t	_status;
		Request			_request;
		Response		_response;

		int				_file_fd;
		

	public:
		const std::string    addr;
		const int            port;
		const Socket*        socket;
		const int            connection_fd;

		Client(std::string addr, int port, const Socket *socket, int fd);
		~Client();
		Server *			findServer(void);
		bool				readFileToResponseBody(void);
		int   				readRequest(void);
		void				resume(int epoll_fd, std::map<int, Client *> *clients);

};

#endif /* CLIENT_HPP */
